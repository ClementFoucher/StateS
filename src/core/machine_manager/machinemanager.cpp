/*
 * Copyright © 2021-2023 Clément Foucher
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

// Current class
#include "machinemanager.h"

// StateS classes
#include "machinestatus.h"
#include "undoredomanager.h"
#include "machinebuilder.h"
#include "graphicmachine.h"
#include "graphicattributes.h"
#include "graphiccomponent.h"
#include "fsm.h"
#include "graphicfsm.h"
#include "fsmsimulator.h"
#include "machineundocommand.h"


/////
// Public global object
unique_ptr<MachineManager> machineManager = make_unique<MachineManager>();


/*
 * Constructor/destructor stuff
 */

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

/*
 * Setters
 */

void MachineManager::setMachine(shared_ptr<Machine> newMachine, shared_ptr<GraphicAttributes> newGraphicAttributes)
{
	this->setMachineInternal(newMachine, newGraphicAttributes);

	// Build machine builder
	this->machineBuilder.reset();
	if (newMachine != nullptr)
	{
		this->machineBuilder = make_shared<MachineBuilder>();
	}

	// Notify machine updated
	this->undoRedoManager->notifyMachineReplaced();
	emit this->machineReplacedEvent();
}

/*
 * Getters
 */

/**
 * @brief MachineManager::getMachine
 * @return The current machine.
 * May be null pointer, so returned object has to be checked for nullness.
 */
shared_ptr<Machine> MachineManager::getMachine() const
{
	return this->machine;
}

/**
 * @brief MachineManager::getMachineStatus
 * @return The current machine status.
 * Is never null. However, content may be irrelevent if
 * current machine is null.
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

shared_ptr<GraphicMachine> MachineManager::getGraphicMachine() const
{
	return this->graphicMachine;
}

shared_ptr<MachineSimulator> MachineManager::getMachineSimulator() const
{
	return this->machineSimulator;
}

/*
 * Actions
 */

void MachineManager::undo()
{
	if (this->undoRedoManager != nullptr)
	{
		this->undoRedoManager->undo();
	}
}

void MachineManager::redo()
{
	if (this->undoRedoManager != nullptr)
	{
		this->undoRedoManager->redo();
	}
}

void MachineManager::notifyMachineEdited(MachineUndoCommand* undoCommand)
{
	if (this->undoRedoMode == false)
	{
		if (undoCommand != nullptr)
		{
			this->undoRedoManager->addUndoCommand(undoCommand);
		}
		else
		{
			this->undoRedoManager->buildAndAddDiffUndoCommand();
		}
	}
	else
	{
		delete undoCommand;
	}
}

void MachineManager::setUndoRedoMode(bool undoRedoMode)
{
	this->undoRedoMode = undoRedoMode;
}

void MachineManager::setSimulationMode(SimulationMode_t newMode)
{
	bool changeOk = true;

	if (newMode == SimulationMode_t::editMode)
	{
		this->machineSimulator.reset();
	}
	else
	{
		// Build simulator
		shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(this->machine);
		if (fsm != nullptr)
		{
			this->machineSimulator = make_shared<FsmSimulator>();
		}
		else
		{
			changeOk = false;
		}
	}

	if (changeOk == true)
	{
		this->currentSimulationMode = newMode;
		emit this->simulationModeChangedEvent(newMode);
	}
}

SimulationMode_t MachineManager::getCurrentSimulationMode() const
{
	return this->currentSimulationMode;
}



/*
 * Slots
 */

void MachineManager::freshMachineAvailableFromUndoRedo(shared_ptr<Machine> updatedMachine, shared_ptr<GraphicAttributes> updatedGraphicAttributes)
{
	this->setMachineInternal(updatedMachine, updatedGraphicAttributes);

	// Clear tool if there was one currently in use
	this->machineBuilder->setTool(MachineBuilderTool_t::none);

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

void MachineManager::logicComponentDeletedEventHandler(componentId_t componentId)
{
	if (this->graphicMachine == nullptr) return;

	this->graphicMachine->removeGraphicComponent(componentId);
}

void MachineManager::graphicComponentNeedsRefreshEventHandler(componentId_t componentId)
{
	auto graphicComponent = this->graphicMachine->getGraphicComponent(componentId);
	if (graphicComponent == nullptr) return;

	graphicComponent->refreshDisplay();
}

/*
 * Private
 */

void MachineManager::setMachineInternal(shared_ptr<Machine> newMachine, shared_ptr<GraphicAttributes> newGraphicAttributes)
{
	// Clear old machine connections so that its destruction will be silent
	this->graphicMachine.reset();
	if (this->machine != nullptr)
	{
		// Event propagation
		disconnect(this->machine.get(), &Machine::machineNameChangedEvent,              this, &MachineManager::machineNameChangedEvent);
		disconnect(this->machine.get(), &Machine::machineInputListChangedEvent,         this, &MachineManager::machineInputListChangedEvent);
		disconnect(this->machine.get(), &Machine::machineOutputListChangedEvent,        this, &MachineManager::machineOutputListChangedEvent);
		disconnect(this->machine.get(), &Machine::machineLocalVariableListChangedEvent, this, &MachineManager::machineLocalVariableListChangedEvent);
		disconnect(this->machine.get(), &Machine::machineConstantListChangedEvent,      this, &MachineManager::machineConstantListChangedEvent);
		// Event handling
		disconnect(this->machine.get(), &Machine::graphicComponentNeedsRefreshEvent,    this, &MachineManager::graphicComponentNeedsRefreshEventHandler);
		disconnect(this->machine.get(), &Machine::componentDeletedEvent,                this, &MachineManager::logicComponentDeletedEventHandler);
	}

	// Update machine
	this->machine = newMachine;

	// Build graphic machine
	shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(newMachine);
	if (fsm != nullptr)
	{
		this->graphicMachine = make_shared<GraphicFsm>();
		this->graphicMachine->build(newGraphicAttributes);
	}

	// Connect new machine

	// Event propagation
	connect(newMachine.get(), &Machine::machineNameChangedEvent,              this, &MachineManager::machineNameChangedEvent);
	connect(newMachine.get(), &Machine::machineOutputListChangedEvent,        this, &MachineManager::machineOutputListChangedEvent);
	connect(newMachine.get(), &Machine::machineLocalVariableListChangedEvent, this, &MachineManager::machineLocalVariableListChangedEvent);
	connect(newMachine.get(), &Machine::machineConstantListChangedEvent,      this, &MachineManager::machineConstantListChangedEvent);
	connect(newMachine.get(), &Machine::machineInputListChangedEvent,         this, &MachineManager::machineInputListChangedEvent);
	// Event handling
	connect(newMachine.get(), &Machine::graphicComponentNeedsRefreshEvent,    this, &MachineManager::graphicComponentNeedsRefreshEventHandler);
	connect(newMachine.get(), &Machine::componentDeletedEvent,                this, &MachineManager::logicComponentDeletedEventHandler);
}
