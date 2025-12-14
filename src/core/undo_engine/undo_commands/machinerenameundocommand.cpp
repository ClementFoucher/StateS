/*
 * Copyright © 2025 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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

// Parent class
#include "machinerenameundocommand.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"


/////
// Constructors/destructors

MachineRenameUndoCommand::MachineRenameUndoCommand(const QString& previousName) :
	StatesUndoCommand(UndoCommandId_t::machineRenameUndoId)
{
	this->previousName = previousName;
}

/////
// Object functions

void MachineRenameUndoCommand::undo()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	// Compute redo
	this->nextName = machine->getName();

	// Apply undo
	machineManager->setUndoRedoMode(true);
	machine->setName(this->previousName);
	machineManager->setUndoRedoMode(false);
}

void MachineRenameUndoCommand::redo()
{
	if (this->firstRedoIgnored == false)
	{
		// Ignore initial redo automatically applied when pushed in the stack
		this->firstRedoIgnored = true;
		return;
	}

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	// Apply redo
	machineManager->setUndoRedoMode(true);
	machine->setName(this->nextName);
	machineManager->setUndoRedoMode(false);

	// Clear redo
	this->nextName = QString();
}
