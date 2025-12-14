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
#include "fsmtransitionconditionsliderpositionchangeundocommand.h"

// StateS classes
#include "machinemanager.h"
#include "graphicfsm.h"
#include "graphicfsmtransition.h"


/////
// Constructors/destructors

FsmTransitionConditionSliderPositionChangeUndoCommand::FsmTransitionConditionSliderPositionChangeUndoCommand(componentId_t componentId) :
	StatesUndoCommand(UndoCommandId_t::fsmTransitionConditionSliderPositionChangeUndoId)
{
	auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicfsm == nullptr) return;

	auto graphicTransition = graphicfsm->getTransition(componentId);
	if (graphicTransition == nullptr) return;


	this->componentId = componentId;
	this->previousSliderPosition = graphicTransition->getConditionLineSliderPosition();
}

/////
// Object functions

void FsmTransitionConditionSliderPositionChangeUndoCommand::undo()
{
	auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicfsm == nullptr) return;

	auto graphicTransition = graphicfsm->getTransition(this->componentId);
	if (graphicTransition == nullptr) return;


	// Compute redo
	this->nextTransitionSliderPosition = graphicTransition->getConditionLineSliderPosition();

	// Apply undo
	machineManager->setUndoRedoMode(true);
	graphicTransition->setConditionLineSliderPosition(this->previousSliderPosition);
	machineManager->setUndoRedoMode(false);

}

void FsmTransitionConditionSliderPositionChangeUndoCommand::redo()
{
	if (this->firstRedoIgnored == false)
	{
		// Ignore initial redo automatically applied when pushed in the stack
		this->firstRedoIgnored = true;
		return;
	}

	auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicfsm == nullptr) return;

	auto graphicTransition = graphicfsm->getTransition(this->componentId);
	if (graphicTransition == nullptr) return;


	// Apply redo
	machineManager->setUndoRedoMode(true);
	graphicTransition->setConditionLineSliderPosition(this->nextTransitionSliderPosition);
	machineManager->setUndoRedoMode(false);
}

bool FsmTransitionConditionSliderPositionChangeUndoCommand::mergeWith(const QUndoCommand* command)
{
	auto otherCommand = dynamic_cast<const FsmTransitionConditionSliderPositionChangeUndoCommand*>(command);
	if (otherCommand == nullptr) return false;


	this->nextTransitionSliderPosition = otherCommand->nextTransitionSliderPosition;

	return true;
}
