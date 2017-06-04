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
#include "include/fsmundocommand.h"

// StateS classes
#include "states.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmgraphicstate.h"


FsmUndoCommand::FsmUndoCommand(shared_ptr<FsmState> state) :
    MachineUndoCommand()
{
	this->undoType = undo_command_id::fsmUndoStateMoveId;
	this->componentRef = state->getName();

	this->previousPosition = state->getGraphicRepresentation()->pos();
}

void FsmUndoCommand::undo()
{
	shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(StateS::getCurrentMachine());

	if (fsm != nullptr)
	{
		switch (this->undoType)
		{
		case undo_command_id::fsmUndoStateMoveId:
		{
			fsm->setInhibitEvents(true);
			shared_ptr<FsmState> state = fsm->getStateByName(this->componentRef);
			FsmGraphicState* stateRepresentation = state->getGraphicRepresentation();
			this->nextPosition = stateRepresentation->pos(); // Save current position for redo
			stateRepresentation->setPos(this->previousPosition);
			fsm->setInhibitEvents(false);
			break;
		}
		default:
			break;
		}
	}
}

void FsmUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(StateS::getCurrentMachine());

		if (fsm != nullptr)
		{
			switch (this->undoType)
			{
			case undo_command_id::fsmUndoStateMoveId:
			{
				fsm->setInhibitEvents(true);
				shared_ptr<FsmState> state = fsm->getStateByName(this->componentRef);
				FsmGraphicState* stateRepresentation = state->getGraphicRepresentation();
				stateRepresentation->setPos(this->nextPosition);
				fsm->setInhibitEvents(false);
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

bool FsmUndoCommand::mergeWith(const QUndoCommand* command)
{
	const FsmUndoCommand* otherCommand = dynamic_cast<const FsmUndoCommand*>(command);
	bool mergeAccepted = false;

	if ( (otherCommand != nullptr) && (otherCommand->getComponentRef() == this->componentRef))
	{
		switch (this->undoType)
		{
		case undo_command_id::fsmUndoStateMoveId:
		{
			this->nextPosition = otherCommand->getNextPosition();
			mergeAccepted = true;
			break;
		}
		default:
			break;
		}
	}

	return mergeAccepted;
}

QString FsmUndoCommand::getComponentRef() const
{
	return this->componentRef;
}

QPointF FsmUndoCommand::getNextPosition() const
{
	return this->nextPosition;
}
