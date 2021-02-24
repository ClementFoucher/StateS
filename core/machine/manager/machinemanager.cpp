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


MachineManager::MachineManager() :
    QObject()
{
	this->machineStatus = shared_ptr<MachineStatus>(new MachineStatus());

	connect(this->machineStatus.get(), &MachineStatus::unsavedFlagChangedEvent, this, &MachineManager::machineUnsavedFlagChangedEventHandler);
}

void MachineManager::build()
{
	MachineUndoCommand::setMachineManager(this->shared_from_this());

	this->undoRedoManager = shared_ptr<UndoRedoManager>(new UndoRedoManager(this->shared_from_this()));

	connect(this->undoRedoManager.get(), &UndoRedoManager::freshMachineAvailableEvent,        this, &MachineManager::freshMachineAvailableFromUndoRedo);
	connect(this->undoRedoManager.get(), &UndoRedoManager::undoActionAvailabilityChangeEvent, this, &MachineManager::undoActionAvailabilityChangeEvent);
	connect(this->undoRedoManager.get(), &UndoRedoManager::redoActionAvailabilityChangeEvent, this, &MachineManager::redoActionAvailabilityChangeEvent);
}

void MachineManager::clear()
{
	// Clear objects that contains a shared_ptr to this
	this->undoRedoManager.reset();
	MachineUndoCommand::setMachineManager(nullptr);
}

void MachineManager::setMachine(shared_ptr<Machine> newMachine)
{
	this->setMachineInternal(newMachine, true);
}

void MachineManager::setViewConfiguration(shared_ptr<ViewConfiguration> viewConfiguration)
{
	this->viewConfiguration = viewConfiguration;
}

shared_ptr<Machine> MachineManager::getMachine() const
{
	return this->machine;
}

shared_ptr<MachineStatus> MachineManager::getMachineStatus() const
{
	return this->machineStatus;
}

shared_ptr<MachineBuilder> MachineManager::getMachineBuilder() const
{
	return this->machineBuilder;
}

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

void MachineManager::addConnection(QMetaObject::Connection connection)
{
	this->connections.append(connection);
}

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
