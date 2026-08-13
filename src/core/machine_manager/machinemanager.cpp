/*
 * Copyright © 2021-2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.txt.
 *
 *
 * This file is part of StateS.
 *
 * StateS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * StateS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "machinemanager.h"

// StateS
#include "machine.h"
#include "fsm.h"
#include "machinestatus.h"
#include "undoredomanager.h"
#include "statesundocommand.h"
#include "machinebuilder.h"
#include "machinesimulator.h"
#include "graphicmachine.h"
#include "simulatedmachine.h"
#include "graphicattributes.h"
#include "graphiccomponent.h"
#include "graphicsimulatedcomponent.h"
#include "graphicfsm.h"


/////
// Public global object
unique_ptr<MachineManager> machineManager = make_unique<MachineManager>();

/////
// Constructors/destructors

MachineManager::MachineManager() :
	QObject()
{
	this->machineStatus   = make_shared<MachineStatus>();
	this->undoRedoManager = make_unique<UndoRedoManager>();

	connect(this->machineStatus.get(), &MachineStatus::unsavedFlagChangedEvent, this, &MachineManager::machineUnsavedFlagChangedEventHandler);

	connect(this->undoRedoManager.get(), &UndoRedoManager::freshMachineAvailableEvent,        this, &MachineManager::freshMachineAvailableFromUndoRedo);
	connect(this->undoRedoManager.get(), &UndoRedoManager::undoActionAvailabilityChangeEvent, this, &MachineManager::undoActionAvailabilityChangedEvent);
	connect(this->undoRedoManager.get(), &UndoRedoManager::redoActionAvailabilityChangeEvent, this, &MachineManager::redoActionAvailabilityChangedEvent);
}

MachineManager::~MachineManager()
{
	// Disconnect all signals before deleting the MachineManager
	// This prevents anarchic calls from machine components being deleted
	// to be transmitted to the machine manager that is already deleted but as
	// the inherited QObject hasn't been yet, signals are not disconnected.

	if (this->machine != nullptr)
	{
		disconnect(this->machine.get(), &Machine::machineNameChangedEvent,         this, &MachineManager::machineNameChangedEvent);
		disconnect(this->machine.get(), &Machine::machineExternalViewChangedEvent, this, &MachineManager::machineExternalViewChangedEvent);

		disconnect(this->machine.get(), &Machine::componentEditedEvent,  this, &MachineManager::componentEditedEventHandler);
		disconnect(this->machine.get(), &Machine::componentDeletedEvent, this, &MachineManager::componentDeletedEventHandler);
	}

	disconnect(this->undoRedoManager.get(), &UndoRedoManager::freshMachineAvailableEvent,        this, &MachineManager::freshMachineAvailableFromUndoRedo);
	disconnect(this->undoRedoManager.get(), &UndoRedoManager::undoActionAvailabilityChangeEvent, this, &MachineManager::undoActionAvailabilityChangedEvent);
	disconnect(this->undoRedoManager.get(), &UndoRedoManager::redoActionAvailabilityChangeEvent, this, &MachineManager::redoActionAvailabilityChangedEvent);

	disconnect(this->machineStatus.get(), &MachineStatus::unsavedFlagChangedEvent, this, &MachineManager::machineUnsavedFlagChangedEventHandler);

	// For destruction order
	this->machineSimulator.reset();
	this->graphicMachine.reset();
	this->machine.reset();
	this->machineBuilder.reset();
	this->machineStatus.reset();
	this->undoRedoManager.reset();
}

/////
// Mutators

void MachineManager::setMachine(shared_ptr<Machine> newMachine, shared_ptr<GraphicAttributes> newGraphicAttributes)
{
	// Close any open mode before changing the machine
	if (this->currentInterfaceMode != InterfaceMode_t::editMode)
	{
		this->setInterfaceMode(InterfaceMode_t::editMode);
	}

	// Update the reference to the machine
	this->setMachineInternal(newMachine, newGraphicAttributes);

	// Build a new machine builder
	this->machineBuilder.reset();
	if (newMachine != nullptr)
	{
		this->machineBuilder = make_shared<MachineBuilder>();
	}

	// Reset the undo/redo manager
	this->undoRedoManager->notifyMachineReplaced();

	// Notify machine updated
	emit this->machineReplacedEvent();
}

void MachineManager::clearMachine()
{
	this->setMachine(nullptr, nullptr);
}

/////
// Accessors

/**
 * @brief MachineManager::getMachine
 * @return The current machine.
 * May be null pointer, so returned object has to be checked for nullness.
 */
shared_ptr<Machine> MachineManager::getMachine() const
{
	return this->machine;
}

shared_ptr<GraphicMachine> MachineManager::getGraphicMachine() const
{
	return this->graphicMachine;
}

shared_ptr<SimulatedMachine> MachineManager::getSimulatedMachine() const
{
	if (this->machineSimulator == nullptr) return nullptr;


	return this->machineSimulator->getSimulatedMachine();
}

/**
 * @brief MachineManager::getMachineStatus
 * @return The current machine status.
 * Is never null. However, its values may be
 * irrelevent if current machine is null.
 */
shared_ptr<MachineStatus> MachineManager::getMachineStatus() const
{
	return this->machineStatus;
}

/**
 * @brief MachineManager::getMachineBuilder
 * @return The machine builder associated to the current machine.
 * May be null pointer, so returned object has to be checked for nullness.
 */
shared_ptr<MachineBuilder> MachineManager::getMachineBuilder() const
{
	return this->machineBuilder;
}

shared_ptr<MachineSimulator> MachineManager::getMachineSimulator() const
{
	return this->machineSimulator;
}

/////
// Undo/redo management

void MachineManager::undo()
{
	this->undoRedoManager->undo();
}

void MachineManager::redo()
{
	this->undoRedoManager->redo();
}

/**
 * @brief MachineManager::notifyMachineAboutToBeDiffEdited
 *        This function has to be called when a diff edit
 *        is about to be performed, *before* the change is
 *        made to the machine. This allows to store the
 *        XML code before change and compute the diff
 *        after the change is made.
 */
void MachineManager::notifyMachineAboutToBeDiffEdited()
{
	this->undoRedoManager->prepareForDiffUndoCommand();
}

/**
 * @brief MachineManager::notifyMachineEdited This is the most
 *        complete call for machine edition: we provide the
 *        undo command, nothing to do but to add it to the
 *        undo stack.
 * @param undoCommand Command to add to the stack.
 */
void MachineManager::notifyMachineEdited(StatesUndoCommand* undoCommand)
{
	if (this->undoRedoMode == false)
	{
		this->undoRedoManager->addUndoCommand(undoCommand);
	}
	else
	{
		delete undoCommand;
	}
}

/**
 * @brief MachineManager::notifyMachineEdited This call for
 *        machine edition does not provide an undo command,
 *        so a diff command will be built. However, we provide
 *        a description so that this command may be merged.
 * @param undoDescription Decription of the undo, used for undo merge.
 */
void MachineManager::notifyMachineEdited(const QString& undoDescription)
{
	if (this->undoRedoMode == false)
	{
		this->undoRedoManager->buildAndAddDiffUndoCommand(undoDescription);
	}
}

/**
 * @brief MachineManager::notifyMachineEdited This call is
 *        the most incomplete of machine edition calls:
 *        we do not provide an undo, nor a description of
 *        the undo. A diff undo command will be built that
 *        can't be merged.
 */
void MachineManager::notifyMachineEdited()
{
	if (this->undoRedoMode == false)
	{
		this->undoRedoManager->buildAndAddDiffUndoCommand();
	}
}

void MachineManager::setUndoRedoMode(bool undoRedoMode)
{
	this->undoRedoMode = undoRedoMode;
}

/////
// Interface mode management

void MachineManager::setInterfaceMode(InterfaceMode_t newMode)
{
	if (newMode == this->currentInterfaceMode) return;


	switch (newMode)
	{
	case InterfaceMode_t::editMode:
		if (this->currentInterfaceMode == InterfaceMode_t::simulateMode)
		{
			this->machineSimulator.reset();
			this->graphicMachine->clearSimulation();
		}
		break;
	case InterfaceMode_t::simulateMode:
	{
		// Build simulator
		this->machineSimulator = make_shared<MachineSimulator>();
		this->machineSimulator->initialize();

		// Build graphic simulated machine
		this->graphicMachine->buildSimulation();

		// Connect simulated machine
		auto simulatedMachine = this->machineSimulator->getSimulatedMachine();
		connect(simulatedMachine.get(), &SimulatedMachine::simulatedComponentUpdatedEvent, this, &MachineManager::simulatedComponentUpdatedEventHandler);

		break;
	}
	case InterfaceMode_t::verifyMode:
		// Nothing to do
		break;
	}

	this->currentInterfaceMode = newMode;
	emit this->interfaceModeChangedEvent(this->currentInterfaceMode);
}

MachineManager::InterfaceMode_t MachineManager::getCurrentInterfaceMode() const
{
	return this->currentInterfaceMode;
}

/////
// Slots

void MachineManager::freshMachineAvailableFromUndoRedo(shared_ptr<Machine> updatedMachine, shared_ptr<GraphicAttributes> updatedGraphicAttributes)
{
	this->setMachineInternal(updatedMachine, updatedGraphicAttributes);

	// Clear tool if there was one currently in use
	this->machineBuilder->setTool(MachineBuilder::Tool_t::none);

	// Notify machine updated
	emit this->machineUpdatedEvent();
}

void MachineManager::machineUnsavedFlagChangedEventHandler()
{
	if (this->machineStatus->getUnsavedFlag() == false)
	{
		this->undoRedoManager->setClean();
	}
}

void MachineManager::componentDeletedEventHandler(componentId_t componentId)
{
	if (this->graphicMachine == nullptr) return;

	this->graphicMachine->removeGraphicComponent(componentId);
}

void MachineManager::componentEditedEventHandler(componentId_t componentId)
{
	auto graphicComponent = this->graphicMachine->getGraphicComponent(componentId);
	if (graphicComponent == nullptr) return;


	graphicComponent->refreshDisplay();
}

void MachineManager::simulatedComponentUpdatedEventHandler(componentId_t componentId)
{
	auto simulatedGraphicComponent = this->graphicMachine->getSimulatedGraphicComponent(componentId);
	if (simulatedGraphicComponent == nullptr) return;


	simulatedGraphicComponent->refreshSimulatedDisplay();
}

/////
// Private functions

void MachineManager::setMachineInternal(shared_ptr<Machine> newMachine, shared_ptr<GraphicAttributes> newGraphicAttributes)
{
	// Cleanup
	if (this->machine != nullptr)
	{
		// Clear old machine connections so that its destruction is silent

		// Event propagation
		disconnect(this->machine.get(), &Machine::machineNameChangedEvent,         this, &MachineManager::machineNameChangedEvent);
		disconnect(this->machine.get(), &Machine::machineExternalViewChangedEvent, this, &MachineManager::machineExternalViewChangedEvent);
		// Event handling
		disconnect(this->machine.get(), &Machine::componentEditedEvent,  this, &MachineManager::componentEditedEventHandler);
		disconnect(this->machine.get(), &Machine::componentDeletedEvent, this, &MachineManager::componentDeletedEventHandler);

		// Destroy the old graphic machine
		this->graphicMachine.reset();

		// Destroy the old machine
		auto oldMachine = this->machine; // Keep a pointer to the old machine so that it is not destroyed immediately
		this->machine = nullptr;         // Make sure the old machine is inaccessible using getMachine
		oldMachine.reset();              // Destroy the old machine
	}

	// Introduce the new machine
	if (newMachine != nullptr)
	{
		this->machine = newMachine;
		this->machine->finalizeLoading();

		// Build graphic machine
		shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(newMachine);
		if (fsm != nullptr)
		{
			this->graphicMachine = make_shared<GraphicFsm>();
			this->graphicMachine->build(newGraphicAttributes);
		}

		// Connect new machine

		// Event propagation
		connect(this->machine.get(), &Machine::machineNameChangedEvent,         this, &MachineManager::machineNameChangedEvent);
		connect(this->machine.get(), &Machine::machineExternalViewChangedEvent, this, &MachineManager::machineExternalViewChangedEvent);
		// Event handling
		connect(this->machine.get(), &Machine::componentEditedEvent,  this, &MachineManager::componentEditedEventHandler);
		connect(this->machine.get(), &Machine::componentDeletedEvent, this, &MachineManager::componentDeletedEventHandler);
	}
}
