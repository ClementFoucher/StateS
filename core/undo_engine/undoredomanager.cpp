/*
 * Copyright © 2020 Clément Foucher
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
#include "diffundocommand.h"
#include "machine.h"
#include "machinestatus.h"
#include "statesui.h"


UndoRedoManager::UndoRedoManager(shared_ptr<StatesUi> ui)
{
	this->statesUi = ui;

	connect(ui.get(), &StatesUi::undoRequestEvent, this, &UndoRedoManager::undo);
	connect(ui.get(), &StatesUi::redoRequestEvent, this, &UndoRedoManager::redo);

	connect(&this->undoStack, &QUndoStack::cleanChanged,   this, &UndoRedoManager::undoStackCleanStateChangeEventHandler);
	connect(&this->undoStack, &QUndoStack::canUndoChanged, this, &UndoRedoManager::undoActionAvailabilityChangeEventHandler);
	connect(&this->undoStack, &QUndoStack::canRedoChanged, this, &UndoRedoManager::redoActionAvailabilityChangeEventHandler);
}

void UndoRedoManager::setMachine(shared_ptr<Machine> newMachine, bool clearStack)
{
	shared_ptr<Machine> oldMachine = this->machine.lock();

	// Cut links with old machine
	if (oldMachine != nullptr)
	{
		disconnect(oldMachine.get(), &Machine::machineEditedWithoutUndoCommandGeneratedEvent, this, &UndoRedoManager::buildDiffUndoCommand);
		disconnect(oldMachine.get(), &Machine::machineEditedWithUndoCommandGeneratedEvent,    this, &UndoRedoManager::addUndoCommand);
	}

	// Replace machine
	this->machine = newMachine;
	if (clearStack == true)
	{
		this->undoStack.clear();
	}

	// Establish links with new machine
	if (newMachine != nullptr)
	{
		connect(newMachine.get(), &Machine::machineEditedWithoutUndoCommandGeneratedEvent, this, &UndoRedoManager::buildDiffUndoCommand);
		connect(newMachine.get(), &Machine::machineEditedWithUndoCommandGeneratedEvent,    this, &UndoRedoManager::addUndoCommand);
	}
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
	shared_ptr<Machine> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = l_machine->getMachineStatus();
		machineStatus->setUnsavedFlag(!clean);
	}
}

void UndoRedoManager::undoActionAvailabilityChangeEventHandler(bool undoAvailable)
{
	shared_ptr<StatesUi> l_statesUi = this->statesUi.lock();
	if (l_statesUi != nullptr)
	{
		l_statesUi->setUndoButtonEnabled(undoAvailable);
	}
}

void UndoRedoManager::redoActionAvailabilityChangeEventHandler(bool redoAvailable)
{
	shared_ptr<StatesUi> l_statesUi = this->statesUi.lock();
	if (l_statesUi != nullptr)
	{
		l_statesUi->setRedoButtonEnabled(redoAvailable);
	}
}

void UndoRedoManager::addUndoCommand(MachineUndoCommand* undoCommand)
{
	this->undoStack.push(undoCommand);

	shared_ptr<Machine> l_machine = this->machine.lock();
	if (l_machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = l_machine->getMachineStatus();
		machineStatus->setUnsavedFlag(true);
	}
}

void UndoRedoManager::buildDiffUndoCommand(MachineUndoCommand::undo_command_id commandId)
{
	shared_ptr<Machine> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		DiffUndoCommand* undoCommand = new DiffUndoCommand(commandId);
		connect(undoCommand, &DiffUndoCommand::applyUndoRedo, this, &UndoRedoManager::freshMachineAvailableEvent);

		this->undoStack.push(undoCommand);

		shared_ptr<MachineStatus> machineStatus = l_machine->getMachineStatus();
		machineStatus->setUnsavedFlag(true);
	}
}
