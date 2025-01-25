/*
 * Copyright © 2014-2025 Clément Foucher
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

// Current class header
#include "fsmsimulator.h"

// Qt classes
#include <QVBoxLayout>
#include <QLabel>
#include <QSignalMapper>
#include <QPushButton>

// StateS classes
#include "machinemanager.h"
#include "clock.h"
#include "fsm.h"
#include "graphicfsm.h"
#include "fsmstate.h"
#include "variable.h"
#include "output.h"
#include "fsmtransition.h"
#include "fsmsimulatedstate.h"
#include "fsmsimulatedtransition.h"
#include "statesexception.h"
#include "exceptiontypes.h"
#include "actiononvariable.h"


FsmSimulator::FsmSimulator() :
    MachineSimulator()
{
	connect(this->clock.get(), &Clock::clockEvent,      this, &FsmSimulator::clockEventHandler);
	connect(this->clock.get(), &Clock::resetLogicEvent, this, &FsmSimulator::resetEventHandler);

	this->reset();
}

void FsmSimulator::build()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto statesIds = fsm->getAllStatesIds();
	for (auto stateId : statesIds)
	{
		auto simulatedState = new FsmSimulatedState(stateId);
		this->simulatedComponents[stateId] = simulatedState;
	}

	auto transitionsIds = fsm->getAllTransitionsIds();
	for (auto transitionId : transitionsIds)
	{
		auto simulatedTransition = new FsmSimulatedTransition(transitionId);

		auto graphicTransition = graphicFsm->getTransition(transitionId);
		if (graphicTransition != nullptr)
		{
			simulatedTransition->setConditionLineSliderPosition(graphicTransition->getConditionLineSliderPosition());
		}

		this->simulatedComponents[transitionId] = simulatedTransition;
	}
}

void FsmSimulator::targetStateSelectionMadeEventHandler(int i)
{
	delete this->targetStateSelector;
	this->targetStateSelector = nullptr;

	this->signalMapper->deleteLater(); // Can't be deleted now as we are in a call from this object
	this->signalMapper = nullptr;

	this->activateTransition(this->potentialTransitionsIds[i]);

	this->potentialTransitionsIds.clear();
}

void FsmSimulator::forceStateActivation(componentId_t stateToActivate)
{
	if (this->activeStateId != nullId)
	{
		auto previousActiveState = dynamic_cast<FsmSimulatedState*>(this->simulatedComponents[this->activeStateId]);
		if (previousActiveState != nullptr)
		{
			previousActiveState->setActive(false);
		}
	}

	this->activeStateId = stateToActivate;

	auto newActiveState = dynamic_cast<FsmSimulatedState*>(this->simulatedComponents[this->activeStateId]);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
	}
}

componentId_t FsmSimulator::getActiveStateId() const
{
	return this->activeStateId;
}

void FsmSimulator::resetEventHandler()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	this->latestTransitionCrossedId = nullId;

	// Disable any active state
	if (this->activeStateId != nullId)
	{
		auto previousActiveState = dynamic_cast<FsmSimulatedState*>(this->simulatedComponents[this->activeStateId]);
		if (previousActiveState != nullptr)
		{
			previousActiveState->setActive(false);
		}
	}

	// Reset inputs and variables to their initial value
	for (shared_ptr<Variable>& sig : fsm->getReadableVariableSignals())
	{
		sig->reinitialize();
	}

	// Then compute outputs: first reset all of them...
	for (shared_ptr<Output>& sig : fsm->getOutputs())
	{
		sig->resetValue();
	}

	// ... then enable initial state which activate its actions
	this->activeStateId = fsm->getInitialStateId();

	auto newActiveState = dynamic_cast<FsmSimulatedState*>(this->simulatedComponents[this->activeStateId]);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
	}
}

void FsmSimulator::clockEventHandler()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	//
	// First reset pulse signals
	auto latestTransitionCrossed = dynamic_cast<FsmSimulatedTransition*>(this->simulatedComponents[this->latestTransitionCrossedId]);
	if (latestTransitionCrossed != nullptr)
	{
		latestTransitionCrossed->deactivateActions();
		this->latestTransitionCrossedId = nullId;
	}

	//
	// Then determine if a transition is to be crossed
	auto currentActiveState = fsm->getState(this->activeStateId);
	if (currentActiveState != nullptr)
	{
		//
		// Look for potential transitions
		QMap<uint, componentId_t> candidateTransitions;
		for (auto transitionId : currentActiveState->getOutgoingTransitionsIds())
		{
			auto transition = fsm->getTransition(transitionId);
			if (transition == nullptr) continue;

			if (transition->getCondition() != nullptr)
			{
				try
				{
					if (transition->getCondition()->isTrue())
					{
						candidateTransitions.insert(candidateTransitions.count(), transition->getId());
					}
				}
				catch (const StatesException& e)
				{
					if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == VariableError_t::variable_is_not_bool) )
					{
						// Transition condition is incorrect, considered false: nothing to do
					}
					else
						throw;
				}
			}
			else
			{
				// Empty conditions are implicitly true
				candidateTransitions.insert(candidateTransitions.count(), transition->getId());
			}
		}

		if (candidateTransitions.count() == 1)
		{
			// One available transition, let's activate it.
			this->activateTransition(candidateTransitions[0]);
		}
		else if (candidateTransitions.count() > 1)
		{
			// If multiple transitions are crossable, ask for wich one to follow.
			// This is just a small instant patch, user should correct his machine.

			this->clock->stop();

			this->targetStateSelector = new QWidget();
			QVBoxLayout* choiceWindowLayout = new QVBoxLayout(this->targetStateSelector);

			QLabel* choiceWindowWarningText = new QLabel(tr("Warning! There are multiple active transitions going out the current state!") + "<br />"
			                                             + tr("This means your FSM is wrong by construction. This should be fixed.") + "<br />"
			                                             + tr("For current simulation, just choose the target state in the following list:"));

			choiceWindowLayout->addWidget(choiceWindowWarningText);

			this->signalMapper = new QSignalMapper();
			auto newActiveState = dynamic_cast<FsmSimulatedState*>(this->simulatedComponents[this->activeStateId]);
			if (newActiveState != nullptr)
			{
				newActiveState->setActive(true);
			}
			connect(this->signalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mappedInt), this, &FsmSimulator::targetStateSelectionMadeEventHandler);

			for (int i = 0 ; i < candidateTransitions.count() ; i++)
			{
				auto candidateTransition = fsm->getTransition(candidateTransitions[i]);
				if (candidateTransition == nullptr) continue;

				auto targetState = fsm->getState(candidateTransition->getTargetStateId());
				QPushButton* button = new QPushButton(targetState->getName());

				this->signalMapper->setMapping(button, i);

				connect(button, &QPushButton::clicked, this->signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
				connect(button, &QPushButton::clicked, this->targetStateSelector, &QWidget::close);

				choiceWindowLayout->addWidget(button);
			}

			this->potentialTransitionsIds = candidateTransitions;
			this->targetStateSelector->show();
		}
		else
		{
			// No transition crossed: look for maintained actions in current state
			for (auto& action : currentActiveState->getActions())
			{
				if ( (action->getActionType() == ActionOnVariableType_t::increment) ||
					 (action->getActionType() == ActionOnVariableType_t::decrement) )
				{
					action->beginAction();
				}
			}
		}
	}
}

void FsmSimulator::activateTransition(componentId_t transitionId)
{
	auto previousActiveState = dynamic_cast<FsmSimulatedState*>(this->simulatedComponents[this->activeStateId]);
	if (previousActiveState != nullptr)
	{
		previousActiveState->setActive(false);
	}

	this->latestTransitionCrossedId = transitionId;

	auto simulatedTransition = dynamic_cast<FsmSimulatedTransition*>(this->simulatedComponents[transitionId]);
	if (simulatedTransition != nullptr)
	{
		simulatedTransition->activateActions();
		this->activeStateId = simulatedTransition->getTargetStateId();
	}

	auto newActiveState = dynamic_cast<FsmSimulatedState*>(this->simulatedComponents[this->activeStateId]);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
	}

	emit stateChangedEvent();
}
