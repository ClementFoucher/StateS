/*
 * Copyright © 2021 Clément Foucher
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


/*
 * Constructor/destructor stuff
 */

MachineManager::MachineManager() :
    QObject()
{
	this->machineStatus = shared_ptr<MachineStatus>(new MachineStatus());

	connect(this->machineStatus.get(), &MachineStatus::unsavedFlagChangedEvent, this, &MachineManager::machineUnsavedFlagChangedEventHandler);
}

/**
 * @brief MachineManager::build
 * Finishes building objects that couldn't be built within
 * the constructor as they require a shared pointer to this.
 * Must be called immediately after the constructor.
 */
void MachineManager::build()
{
	MachineUndoCommand::setMachineManager(this->shared_from_this());

	this->undoRedoManager = make_shared<UndoRedoManager>(this->shared_from_this());

	connect(this->undoRedoManager.get(), &UndoRedoManager::freshMachineAvailableEvent,        this, &MachineManager::freshMachineAvailableFromUndoRedo);
	connect(this->undoRedoManager.get(), &UndoRedoManager::undoActionAvailabilityChangeEvent, this, &MachineManager::undoActionAvailabilityChangeEvent);
	connect(this->undoRedoManager.get(), &UndoRedoManager::redoActionAvailabilityChangeEvent, this, &MachineManager::redoActionAvailabilityChangeEvent);
}

/**
 * @brief MachineManager::clear
 * Clears objects that have a shared pointer to this,
 * as simply reseting the smart pointer to this would
 * not clear all references.
 * This must be called before deleting the main smart
 * pointer to this.
 */
void MachineManager::clear()
{
	// Clear objects that contains a shared_ptr to this
	this->undoRedoManager.reset();
	MachineUndoCommand::setMachineManager(nullptr);
}

/*
 * Setters
 */

void MachineManager::setMachine(shared_ptr<Machine> newMachine)
{
	this->setMachineInternal(newMachine, true);
}

void MachineManager::setViewConfiguration(shared_ptr<ViewConfiguration> viewConfiguration)
{
	this->viewConfiguration = viewConfiguration;
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
 * current machiene is null.
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

/**
 * @brief MachineManager::getViewConfiguration
 * @return The view configuration.
 * This object is only temporary: it is set only when dealing with save files:
 * - When saving, it is set by the scene widget and used by the XML writer
 * - When loading, it is set by the XML parser and used by the scene widget.
 * In any other context, this object content is irrelevant, and may even be null.
 */
shared_ptr<ViewConfiguration> MachineManager::getViewConfiguration() const
{
	return this->viewConfiguration;
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

void MachineManager::updateViewConfiguration()
{
	emit machineViewUpdateRequestedEvent();
}

/**
 * @brief MachineManager::addConnection
 * This function is used by objects to indicate they have
 * a connection with the machine.
 * This allows the machine manager to cut all connections to
 * the machine when it changes to avoid event propagation
 * from an obsolete machine while replacing it.
 * In most cases, things would be OK without it as the machine
 * destruction causes connections deletion, but this is a
 * safety as sometime the complex signal web may be triggered
 * by object deletion. This ensures all connections are cut
 * before object is deleted to prevent it.
 * @param connection
 */
void MachineManager::addConnection(QMetaObject::Connection connection)
{
	this->connections.append(connection);
}

/*
 * Slots
 */

void MachineManager::freshMachineAvailableFromUndoRedo(shared_ptr<Machine> updatedMachine)
{
	this->setMachineInternal(updatedMachine, false);
}

void MachineManager::machineUnsavedFlagChangedEventHandler()
{
	if (this->machineStatus->getUnsavedFlag() == false)
	{
		this->undoRedoManager->setClean();
	}
}

/*
 * Private
 */

void MachineManager::setMachineInternal(shared_ptr<Machine> newMachine, bool isNewMachine)
{
	// Cut links with old machine
	for (QMetaObject::Connection connection : this->connections)
	{
		disconnect(connection);
	}
	this->connections.clear();

	// Update machine
	this->machine.reset(); // TODO : Not sure why this has to be done first ??? If not, causes a crash when undo stack is not empty...
	this->machine = newMachine;

	this->machineBuilder.reset();
	if (newMachine != nullptr)
	{
		this->machineBuilder = shared_ptr<MachineBuilder>(new MachineBuilder());
	}

	// Notify machine updated
	emit this->machineUpdatedEvent(isNewMachine);
}
