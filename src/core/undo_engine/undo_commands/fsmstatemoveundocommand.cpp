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
#include "fsmstatemoveundocommand.h"

// StateS classes
#include "machinemanager.h"
#include "graphicfsm.h"
#include "graphicfsmstate.h"


/////
// Constructors/destructors

FsmStateMoveUndoCommand::FsmStateMoveUndoCommand(const QString& description, componentId_t componentId) :
	StatesUndoCommand(UndoCommandId_t::fsmStateMoveUndoId, description)
{
	auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicfsm == nullptr) return;

	auto graphicState = graphicfsm->getState(componentId);
	if (graphicState == nullptr) return;


	this->previousStatesPositions[componentId] = graphicState->pos();
}

/////
// Object functions

void FsmStateMoveUndoCommand::undo()
{
	auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicfsm == nullptr) return;


	machineManager->setUndoRedoMode(true);
	for (auto& stateId : this->previousStatesPositions.keys())
	{
		auto graphicState = graphicfsm->getState(stateId);
		if (graphicState == nullptr) continue;

		// Compute redo
		this->nextStatesPositions[stateId] = graphicState->pos();

		// Apply undo
		graphicState->setPos(this->previousStatesPositions.value(stateId));
	}
	machineManager->setUndoRedoMode(false);
}

void FsmStateMoveUndoCommand::redo()
{
	if (this->firstRedoIgnored == false)
	{
		// Ignore initial redo automatically applied when pushed in the stack
		this->firstRedoIgnored = true;
		return;
	}

	auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicfsm == nullptr) return;


	// Apply redo
	machineManager->setUndoRedoMode(true);
	for (auto& stateId : this->nextStatesPositions.keys())
	{
		auto graphicState = graphicfsm->getState(stateId);
		if (graphicState == nullptr) continue;


		graphicState->setPos(this->nextStatesPositions.value(stateId));
	}
	machineManager->setUndoRedoMode(false);

	// Clear redo
	this->nextStatesPositions.clear();
}

bool FsmStateMoveUndoCommand::mergeWith(const QUndoCommand* command)
{
	auto otherCommand = dynamic_cast<const FsmStateMoveUndoCommand*>(command);
	if (otherCommand == nullptr) return false;

	if (otherCommand->text() != this->text()) return false;


	for (auto& stateId : otherCommand->previousStatesPositions.keys())
	{
		if (this->previousStatesPositions.keys().contains(stateId) == false)
		{
			this->previousStatesPositions[stateId] = otherCommand->previousStatesPositions.value(stateId);
		}
	}

	return true;
}
