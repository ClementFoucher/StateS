/*
 * Copyright © 2020-2025 Clément Foucher
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
	DiffUndoCommand::clearXmlRepresentation();
}

void UndoRedoManager::redo()
{
	this->undoStack.redo();
	DiffUndoCommand::clearXmlRepresentation();
}

void UndoRedoManager::setClean()
{
	this->undoStack.setClean();
}

void UndoRedoManager::addUndoCommand(StatesUndoCommand* undoCommand)
{
	// We have to make this test before pushing to the stack
	// as it will take ownership and invalidate the pointer
	bool isDiffUndoCommand = dynamic_cast<DiffUndoCommand*>(undoCommand) != nullptr;

	this->undoStack.push(undoCommand);

	if (isDiffUndoCommand == false)
	{
		// Avoid invalidating for diff undo commands as XML
		// representation is updated as part of building the
		// command. This avoids rebuilding the XML if two
		// consecutive changes use diff undo commands
		DiffUndoCommand::clearXmlRepresentation();
	}
}

void UndoRedoManager::buildAndAddDiffUndoCommand(const QString& undoDescription)
{
	DiffUndoCommand* undoCommand = new DiffUndoCommand(undoDescription);

	connect(undoCommand, &DiffUndoCommand::applyUndoRedo, this, &UndoRedoManager::freshMachineAvailableEvent);
	this->addUndoCommand(undoCommand);
}

void UndoRedoManager::prepareForDiffUndoCommand()
{
	DiffUndoCommand::buildXmlRepresentation();
}

void UndoRedoManager::notifyMachineReplaced()
{
	this->undoStack.clear();
	DiffUndoCommand::clearXmlRepresentation();
}

void UndoRedoManager::undoStackCleanStateChangeEventHandler(bool clean)
{
	auto machineStatus = machineManager->getMachineStatus();
	machineStatus->setUnsavedFlag(!clean);
}
