/*
 * Copyright © 2017-2025 Clément Foucher
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
 * along with StateS. If not, see <http://www.gnu.org/licenses/>.
 */

// Parent class
#include "machineundocommand.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"


/////
// Constructors/destructors

MachineUndoCommand::MachineUndoCommand(const QString& previousName)
{
	this->undoType = UndoCommandId_t::machineUndoRenameId;
	this->previousName = previousName;
}

/////
// Object functions

void MachineUndoCommand::undo()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	switch (this->undoType)
	{
	case UndoCommandId_t::machineUndoRenameId:
	{
		this->nextName = machine->getName();

		machineManager->setUndoRedoMode(true);
		machine->setName(this->previousName);
		machineManager->setUndoRedoMode(false);

		break;
	}
	default:
		break;
	}
}

void MachineUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		auto machine = machineManager->getMachine();
		if (machine != nullptr)
		{
			switch (this->undoType)
			{
			case UndoCommandId_t::machineUndoRenameId:
			{
				this->nextName = QString();

				machineManager->setUndoRedoMode(true);
				machine->setName(this->nextName);
				machineManager->setUndoRedoMode(false);

				break;
			}
			default:
				break;
			}
		}
	}
	else
	{
		// Ignore initial redo automatically applied when pushed in the stack
		this->firstRedoIgnored = true;
	}
}

int MachineUndoCommand::id() const
{
	return (int)this->undoType;
}
