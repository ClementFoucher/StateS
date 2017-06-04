/*
 * Copyright © 2017 Clément Foucher
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
#include "states.h"
#include "fsm.h"


MachineUndoCommand::MachineUndoCommand()
{
	this->firstRedoIgnored = false;
}

MachineUndoCommand::MachineUndoCommand(const QString& previousName) :
    MachineUndoCommand()
{
	this->undoType = machineUndoRenameId;
	this->previousName = previousName;
}

void MachineUndoCommand::undo()
{
	shared_ptr<Machine> machine = StateS::getCurrentMachine();
	if (machine != nullptr)
	{
		switch (this->undoType)
		{
		case undo_command_id::machineUndoRenameId:
		{
			machine->setInhibitEvents(true);
			this->nextName = machine->getName();
			machine->setName(this->previousName);
			machine->setInhibitEvents(false);
			break;
		}
		default:
			break;
		}
	}
}

void MachineUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		shared_ptr<Machine> machine = StateS::getCurrentMachine();
		if (machine != nullptr)
		{
			switch (this->undoType)
			{
			case undo_command_id::machineUndoRenameId:
			{
				machine->setInhibitEvents(true);
				machine->setName(this->nextName);
				this->nextName = QString::null;
				machine->setInhibitEvents(false);
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
	return this->undoType;
}
