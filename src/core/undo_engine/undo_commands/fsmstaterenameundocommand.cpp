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

// Current class header
#include "fsmstaterenameundocommand.h"

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "fsmstate.h"


/////
// Constructors/destructors

FsmStateRenameUndoCommand::FsmStateRenameUndoCommand(componentId_t componentId, const QString& previousStateName) :
	StatesUndoCommand(UndoCommandId_t::fsmStateRenamedUndoId)
{
	this->componentId = componentId;
	this->previousStateName = previousStateName;
}

/////
// Object functions

void FsmStateRenameUndoCommand::undo()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->componentId);
	if (logicState == nullptr) return;


	// Compute redo
	this->nextStateName = logicState->getName();

	// Apply undo
	machineManager->setUndoRedoMode(true);
	logicState->setName(this->previousStateName);
	machineManager->setUndoRedoMode(false);
}

void FsmStateRenameUndoCommand::redo()
{
	if (this->firstRedoIgnored == false)
	{
		// Ignore initial redo automatically applied when pushed in the stack
		this->firstRedoIgnored = true;
		return;
	}

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->componentId);
	if (logicState == nullptr) return;


	// Apply redo
	machineManager->setUndoRedoMode(true);
	logicState->setName(this->nextStateName);
	machineManager->setUndoRedoMode(false);

	// Clear redo
	this->nextStateName = QString();
}

bool FsmStateRenameUndoCommand::mergeWith(const QUndoCommand* command)
{
	auto otherCommand = dynamic_cast<const FsmStateRenameUndoCommand*>(command);
	if (otherCommand == nullptr) return false;


	this->nextStateName = otherCommand->nextStateName;

	return true;
}
