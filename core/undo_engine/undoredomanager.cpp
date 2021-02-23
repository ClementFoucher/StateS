/*
 * Copyright © 2020-2021 Clément Foucher
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
#include "undoredomanager.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "diffundocommand.h"
#include "machinestatus.h"


UndoRedoManager::UndoRedoManager(shared_ptr<MachineManager> machineManager)
{
	this->machineManager = machineManager;

	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &UndoRedoManager::machineUpdatedEventHandler);

	connect(&this->undoStack, &QUndoStack::cleanChanged,   this, &UndoRedoManager::undoStackCleanStateChangeEventHandler);
	connect(&this->undoStack, &QUndoStack::canUndoChanged, this, &UndoRedoManager::undoActionAvailabilityChangeEvent);
	connect(&this->undoStack, &QUndoStack::canRedoChanged, this, &UndoRedoManager::redoActionAvailabilityChangeEvent);
}

void UndoRedoManager::clearStack()
{
	this->undoStack.clear();
}

void UndoRedoManager::setClean()
{
	this->undoStack.setClean();
}

void UndoRedoManager::undo()
{
	this->undoStack.undo();
}

void UndoRedoManager::redo()
{
	this->undoStack.redo();
}

void UndoRedoManager::undoStackCleanStateChangeEventHandler(bool clean)
{
	this->setMachineUnsavedFlag(!clean);
}

void UndoRedoManager::machineUpdatedEventHandler(bool isNewMachine)
{
	if (isNewMachine == true)
	{
		this->undoStack.clear();
	}

	// Establish links with new machine
	shared_ptr<Machine> newMachine = this->machineManager->getMachine();
	if (newMachine != nullptr)
	{
		this->machineManager->addConnection(connect(newMachine.get(), &Machine::machineEditedWithoutUndoCommandGeneratedEvent, this, &UndoRedoManager::buildDiffUndoCommand));
		this->machineManager->addConnection(connect(newMachine.get(), &Machine::machineEditedWithUndoCommandGeneratedEvent,    this, &UndoRedoManager::addUndoCommand));
	}
}

void UndoRedoManager::addUndoCommand(MachineUndoCommand* undoCommand)
{
	this->undoStack.push(undoCommand);
	this->setMachineUnsavedFlag(false);
}

void UndoRedoManager::buildDiffUndoCommand(MachineUndoCommand::undo_command_id commandId)
{
	DiffUndoCommand* undoCommand = new DiffUndoCommand(commandId);
	connect(undoCommand, &DiffUndoCommand::applyUndoRedo, this, &UndoRedoManager::freshMachineAvailableEvent);

	this->undoStack.push(undoCommand);

	this->setMachineUnsavedFlag(true);
}

void UndoRedoManager::setMachineUnsavedFlag(bool unsaved)
{
	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	machineStatus->setUnsavedFlag(unsaved);
}
