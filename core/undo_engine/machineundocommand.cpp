/*
 * Copyright © 2017-2021 Clément Foucher
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
#include "diffundocommand.h"


shared_ptr<MachineManager> MachineUndoCommand::machineManager;

void MachineUndoCommand::setMachineManager(shared_ptr<MachineManager> machineManager)
{
	MachineUndoCommand::machineManager = machineManager;
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, &MachineUndoCommand::machineUpdatedEventHandler);
}

void MachineUndoCommand::machineUpdatedEventHandler(bool machineHasChanged)
{
	// If machine has changed, its XML representation must be rebuilt
	if (machineHasChanged == true)
	{
		DiffUndoCommand::updateXmlRepresentation();
	}
}

MachineUndoCommand::MachineUndoCommand()
{
	this->firstRedoIgnored = false;
}

MachineUndoCommand::MachineUndoCommand(const QString& previousName) :
    MachineUndoCommand()
{
	this->undoType = machineUndoRenameId;
	this->previousName = previousName;

	DiffUndoCommand::updateXmlRepresentation();
}

void MachineUndoCommand::undo()
{
	shared_ptr<Machine> l_machine = MachineUndoCommand::machineManager->getMachine();
	if (l_machine != nullptr)
	{
		switch (this->undoType)
		{
		case undo_command_id::machineUndoRenameId:
		{
			l_machine->setInhibitEvents(true);
			this->nextName = l_machine->getName();
			l_machine->setName(this->previousName);
			l_machine->setInhibitEvents(false);
			break;
		}
		default:
			break;
		}

		DiffUndoCommand::updateXmlRepresentation();
	}
}

void MachineUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		shared_ptr<Machine> l_machine = MachineUndoCommand::machineManager->getMachine();
		if (l_machine != nullptr)
		{
			switch (this->undoType)
			{
			case undo_command_id::machineUndoRenameId:
			{
				l_machine->setInhibitEvents(true);
				l_machine->setName(this->nextName);
				this->nextName = QString();
				l_machine->setInhibitEvents(false);
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

	DiffUndoCommand::updateXmlRepresentation();
}

int MachineUndoCommand::id() const
{
	return this->undoType;
}
