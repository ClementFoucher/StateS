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

// C++ classes
#include <memory>
using namespace std;

// Current class header
#include "fsmundocommand.h"

// StateS classes
#include "machinemanager.h"
#include "graphicmachine.h"
#include "fsm.h"
#include "fsmstate.h"
#include "graphicfsm.h"
#include "graphicfsmstate.h"
#include "graphicfsmtransition.h"


FsmUndoCommand::FsmUndoCommand(UndoCommandId_t undoType, componentId_t componentId) :
    MachineUndoCommand()
{
	if ( (undoType != UndoCommandId_t::fsmUndoStateMoveId) && (undoType != UndoCommandId_t::fsmUndoTransitionConditionSliderPositionChangeId) ) return;

	auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicfsm == nullptr) return;

	switch (undoType)
	{
	case UndoCommandId_t::fsmUndoStateMoveId:
	{
		auto graphicState = graphicfsm->getState(componentId);
		if (graphicState == nullptr) return;

		this->componentId = componentId;
		this->previousStatePosition = graphicState->pos();

		break;
	}
	case UndoCommandId_t::fsmUndoTransitionConditionSliderPositionChangeId:
	{
		auto graphicTransition = graphicfsm->getTransition(componentId);
		if (graphicTransition == nullptr) return;

		this->componentId = componentId;
		this->previousTransitionSliderPosition = graphicTransition->getConditionLineSliderPosition();

		break;
	}
	default:
		break;
	}

	// Only affect undo type at the end in case there was an error before:
	// in that case, undo type remains default, thus undo/redo won't be applyed.
	this->undoType = undoType;
}

FsmUndoCommand::FsmUndoCommand(componentId_t componentId, const QString& previousStateName)
{
	this->undoType = UndoCommandId_t::fsmUndoStateRenamedId;
	this->componentId = componentId;
	this->previousStateName = previousStateName;
}

void FsmUndoCommand::undo()
{
	switch (this->undoType)
	{
	case UndoCommandId_t::fsmUndoStateMoveId:
	{
		auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
		if (graphicfsm == nullptr) return;

		auto graphicState = graphicfsm->getState(this->componentId);
		if (graphicState == nullptr) return;

		// Save current position for redo
		this->nextStatePosition = graphicState->pos();

		// Apply undo
		machineManager->setUndoRedoMode(true);
		graphicState->setPos(this->previousStatePosition);
		machineManager->setUndoRedoMode(false);

		break;
	}
	case UndoCommandId_t::fsmUndoTransitionConditionSliderPositionChangeId:
	{
		auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
		if (graphicfsm == nullptr) return;

		auto graphicTransition = graphicfsm->getTransition(this->componentId);
		if (graphicTransition == nullptr) return;

		// Save current position for redo
		this->nextTransitionSliderPosition = graphicTransition->getConditionLineSliderPosition();

		// Apply undo
		machineManager->setUndoRedoMode(true);
		graphicTransition->setConditionLineSliderPosition(this->previousTransitionSliderPosition);
		machineManager->setUndoRedoMode(false);

		break;
	}
	case UndoCommandId_t::fsmUndoStateRenamedId:
	{
		auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
		if (fsm == nullptr) return;

		auto logicState = fsm->getState(this->componentId);
		if (logicState == nullptr) return;

		// Save current name  for redo
		this->nextStateName = logicState->getName();

		// Apply undo
		machineManager->setUndoRedoMode(true);
		logicState->setName(this->previousStateName);
		machineManager->setUndoRedoMode(false);

		break;
	}
	default:
		break;
	}
}

void FsmUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
		if (fsm == nullptr) return;

		auto graphicfsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
		if (graphicfsm == nullptr) return;

		switch (this->undoType)
		{
		case UndoCommandId_t::fsmUndoStateMoveId:
		{
			auto graphicState = graphicfsm->getState(this->componentId);
			if (graphicState == nullptr) return;

			// Apply redo
			machineManager->setUndoRedoMode(true);
			graphicState->setPos(this->nextStatePosition);
			machineManager->setUndoRedoMode(false);

			break;
		}
		case UndoCommandId_t::fsmUndoTransitionConditionSliderPositionChangeId:
		{
			auto graphicTransition = graphicfsm->getTransition(this->componentId);
			if (graphicTransition == nullptr) return;

			// Apply redo
			machineManager->setUndoRedoMode(true);
			graphicTransition->setConditionLineSliderPosition(this->nextTransitionSliderPosition);
			machineManager->setUndoRedoMode(false);

			break;
		}
		case UndoCommandId_t::fsmUndoStateRenamedId:
		{
			auto logicState = fsm->getState(this->componentId);
			if (logicState == nullptr) return;

			// Apply redo
			machineManager->setUndoRedoMode(true);
			logicState->setName(this->nextStateName);
			machineManager->setUndoRedoMode(false);

			break;
		}
		default:
			break;
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
	if (otherCommand == nullptr) return false;
	if (otherCommand->componentId != this->componentId) return false;

	bool mergeAccepted = false;

	switch (this->undoType)
	{
	case UndoCommandId_t::fsmUndoStateMoveId:
	{
		this->nextStatePosition = otherCommand->nextStatePosition;
		mergeAccepted = true;
		break;
	}
	case UndoCommandId_t::fsmUndoTransitionConditionSliderPositionChangeId:
	{
		this->nextTransitionSliderPosition = otherCommand->nextTransitionSliderPosition;
		mergeAccepted = true;
		break;
	}
	case UndoCommandId_t::fsmUndoStateRenamedId:
	{
		this->nextStateName = otherCommand->nextStateName;
		mergeAccepted = true;
		break;
	}
	default:
		break;
	}

	return mergeAccepted;
}
