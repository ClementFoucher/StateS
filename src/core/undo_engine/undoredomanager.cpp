/*
 * Copyright © 2020-2023 Clément Foucher
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


UndoRedoManager::UndoRedoManager()
{
	connect(&this->undoStack, &QUndoStack::cleanChanged,   this, &UndoRedoManager::undoStackCleanStateChangeEventHandler);
	connect(&this->undoStack, &QUndoStack::canUndoChanged, this, &UndoRedoManager::undoActionAvailabilityChangeEvent);
	connect(&this->undoStack, &QUndoStack::canRedoChanged, this, &UndoRedoManager::redoActionAvailabilityChangeEvent);
}

void UndoRedoManager::undo()
{
	this->undoStack.undo();
	DiffUndoCommand::updateXmlRepresentation();
}

void UndoRedoManager::redo()
{
	this->undoStack.redo();
	DiffUndoCommand::updateXmlRepresentation();
}

void UndoRedoManager::clearStack()
{
	this->undoStack.clear();
}

void UndoRedoManager::setClean()
{
	this->undoStack.setClean();
}

void UndoRedoManager::addUndoCommand(MachineUndoCommand* undoCommand)
{
	this->undoStack.push(undoCommand);

	auto machineStatus = machineManager->getMachineStatus();
	machineStatus->setUnsavedFlag(true);

	DiffUndoCommand::updateXmlRepresentation();
}

void UndoRedoManager::buildAndAddDiffUndoCommand()
{
	DiffUndoCommand* undoCommand = new DiffUndoCommand();
	connect(undoCommand, &DiffUndoCommand::applyUndoRedo, this, &UndoRedoManager::freshMachineAvailableEvent);

	this->addUndoCommand(undoCommand);
}

void UndoRedoManager::notifyMachineReplaced()
{
	this->undoStack.clear();
	DiffUndoCommand::updateXmlRepresentation();
}

void UndoRedoManager::undoStackCleanStateChangeEventHandler(bool clean)
{
	auto machineStatus = machineManager->getMachineStatus();
	machineStatus->setUnsavedFlag(!clean);
}
