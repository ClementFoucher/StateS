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
#include "simulatedfsm.h"

// Qt classes
#include <QVBoxLayout>
#include <QLabel>
#include <QSignalMapper>
#include <QPushButton>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "clock.h"
#include "graphicfsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "variable.h"
#include "simulatedfsmstate.h"
#include "simulatedfsmtransition.h"
#include "actiononvariable.h"
#include "equation.h"


SimulatedFsm::SimulatedFsm() :
    SimulatedMachine()
{
	connect(this->clock.get(), &Clock::clockAboutToTickEvent,    this, &SimulatedFsm::clockAboutToTickEventHandler);
	connect(this->clock.get(), &Clock::clockPrepareActionsEvent, this, &SimulatedFsm::clockPrepareActionsEventHandler);
	connect(this->clock.get(), &Clock::clockEvent,               this, &SimulatedFsm::clockEventHandler);
	connect(this->clock.get(), &Clock::resetLogicEvent,          this, &SimulatedFsm::resetEventHandler);

	this->reset();
}

void SimulatedFsm::build()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;


	for (const auto& stateId : fsm->getAllStatesIds())
	{
		auto simulatedState = new SimulatedFsmState(stateId);
		this->simulatedComponents[stateId] = simulatedState;
	}

	for (const auto& transitionId : fsm->getAllTransitionsIds())
	{
		auto simulatedTransition = new SimulatedFsmTransition(transitionId);

		auto graphicTransition = graphicFsm->getTransition(transitionId);
		if (graphicTransition != nullptr)
		{
			simulatedTransition->setConditionLineSliderPosition(graphicTransition->getConditionLineSliderPosition());
		}

		this->simulatedComponents[transitionId] = simulatedTransition;
	}
}

void SimulatedFsm::targetStateSelectionMadeEventHandler(int i)
{
	delete this->targetStateSelector;
	this->targetStateSelector = nullptr;

	this->signalMapper->deleteLater(); // Can't be deleted now as we are in a call from this object
	this->signalMapper = nullptr;

	this->crossTransition(this->potentialTransitionsIds[i]);

	this->potentialTransitionsIds.clear();
}

void SimulatedFsm::forceStateActivation(componentId_t stateToActivate)
{
	if (this->activeStateId != nullId)
	{
		auto previousActiveState = dynamic_cast<SimulatedFsmState*>(this->simulatedComponents[this->activeStateId]);
		if (previousActiveState != nullptr)
		{
			previousActiveState->setActive(false);
			this->deactivateStateActions(this->activeStateId);
		}
	}

	this->activeStateId = stateToActivate;

	auto newActiveState = dynamic_cast<SimulatedFsmState*>(this->simulatedComponents[this->activeStateId]);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
		this->activateStateActions(this->activeStateId, true);
	}
}

componentId_t SimulatedFsm::getActiveStateId() const
{
	return this->activeStateId;
}

void SimulatedFsm::resetEventHandler()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	this->latestTransitionCrossedId = nullId;

	// Disable any active state
	if (this->activeStateId != nullId)
	{
		auto previousActiveState = dynamic_cast<SimulatedFsmState*>(this->simulatedComponents[this->activeStateId]);
		if (previousActiveState != nullptr)
		{
			previousActiveState->setActive(false);
			this->deactivateStateActions(this->activeStateId);
		}
	}

	// Reset inputs and internal variables to their initial value
	for (auto& variableId : fsm->getInputVariablesIds())
	{
		auto variable = fsm->getVariable(variableId);
		if (variable == nullptr) continue;


		variable->reinitialize();
	}
	for (auto& variableId : fsm->getInternalVariablesIds())
	{
		auto variable = fsm->getVariable(variableId);
		if (variable == nullptr) continue;


		variable->reinitialize();
	}

	// Then compute outputs: first reset all of them...
	for (auto& variableId : fsm->getOutputVariablesIds())
	{
		auto variable = fsm->getVariable(variableId);
		if (variable == nullptr) continue;


		variable->reinitialize();
	}

	// ... then enable initial state which activate its actions
	this->activeStateId = fsm->getInitialStateId();

	auto newActiveState = dynamic_cast<SimulatedFsmState*>(this->simulatedComponents[this->activeStateId]);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
		this->activateStateActions(this->activeStateId, true);
	}
}

void SimulatedFsm::clockAboutToTickEventHandler()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	this->transitionToBeCrossedId = nullId;

	// Determine if a transition is to be crossed
	auto currentActiveState = fsm->getState(this->activeStateId);
	if (currentActiveState != nullptr)
	{
		//
		// Look for potential transitions
		QMap<uint, componentId_t> candidateTransitions;
		for (const auto& transitionId : currentActiveState->getOutgoingTransitionsIds())
		{
			auto transition = fsm->getTransition(transitionId);
			if (transition == nullptr) continue;

			auto condition = transition->getCondition();
			if (condition != nullptr)
			{
				if (condition->isTrue())
				{
					candidateTransitions.insert(candidateTransitions.count(), transition->getId());
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
			this->transitionToBeCrossedId = candidateTransitions[0];
		}
		else if (candidateTransitions.count() > 1)
		{
			// If multiple transitions are crossable, ask for wich one to follow.
			// This is just a small instant patch, user should correct his machine.

			this->suspend();

			this->targetStateSelector = new QWidget();
			QVBoxLayout* choiceWindowLayout = new QVBoxLayout(this->targetStateSelector);

			QLabel* choiceWindowWarningText = new QLabel(tr("Warning! There are multiple active transitions going out the current state!") + "<br />"
														 + tr("This means your FSM is wrong by construction. This should be fixed.") + "<br />"
														 + tr("For current simulation, just choose the target state in the following list:"));

			choiceWindowLayout->addWidget(choiceWindowWarningText);

			this->signalMapper = new QSignalMapper();
			connect(this->signalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mappedInt), this, &SimulatedFsm::targetStateSelectionMadeEventHandler);

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
	}
}

void SimulatedFsm::clockPrepareActionsEventHandler()
{
	// First reset actions from the previous cycle
	if (this->latestTransitionCrossedId != nullId)
	{
		this->deactivateTransitionActions(this->latestTransitionCrossedId, true);
	}

	// Then prepare for next actions
	if (this->transitionToBeCrossedId != nullId)
	{
		this->activateTransitionActions(this->transitionToBeCrossedId, true);
	}
}

void SimulatedFsm::clockEventHandler()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	//
	// First reset pulse variables
	if (this->latestTransitionCrossedId != nullId)
	{
		this->deactivateTransitionActions(this->latestTransitionCrossedId, false);
		this->latestTransitionCrossedId = nullId;
	}

	// Look for memorized actions in current state
	this->activateStateActions(this->activeStateId, false);

	//
	// Then determine if a transition is to be crossed
	auto currentActiveState = fsm->getState(this->activeStateId);
	if (this->transitionToBeCrossedId != nullId)
	{
		this->crossTransition(this->transitionToBeCrossedId);
	}
}

void SimulatedFsm::crossTransition(componentId_t transitionId)
{
	auto previousActiveState = dynamic_cast<SimulatedFsmState*>(this->simulatedComponents[this->activeStateId]);
	if (previousActiveState != nullptr)
	{
		previousActiveState->setActive(false);
		this->deactivateStateActions(this->activeStateId);
	}

	this->latestTransitionCrossedId = transitionId;

	auto simulatedTransition = dynamic_cast<SimulatedFsmTransition*>(this->simulatedComponents[transitionId]);
	if (simulatedTransition != nullptr)
	{
		this->activateTransitionActions(transitionId, false);
		this->activeStateId = simulatedTransition->getTargetStateId();
	}

	auto newActiveState = dynamic_cast<SimulatedFsmState*>(this->simulatedComponents[this->activeStateId]);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
		this->activateStateActions(this->activeStateId, true);
	}

	emit stateChangedEvent();
}

void SimulatedFsm::activateStateActions(componentId_t actuatorId, bool isFirstActivation)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuatorComponent = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(actuatorId));
	if (actuatorComponent == nullptr) return;


	for (const auto& action : actuatorComponent->getActions())
	{
		if (action->isActionMemorized() == true)
		{
			if (this->memorizedStateActionBehavior == SimulationBehavior_t::immediately)
			{
				action->beginAction();
			}
			else if (isFirstActivation == false)
			{
				action->beginAction();
			}
		}
		else
		{
			if (this->continuousStateActionBehavior == SimulationBehavior_t::immediately)
			{
				action->beginAction();
			}
			else if (isFirstActivation == false)
			{
				action->beginAction();
			}
		}
	}
}

void SimulatedFsm::activateTransitionActions(componentId_t actuatorId, bool isPreparation)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuatorComponent = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(actuatorId));
	if (actuatorComponent == nullptr) return;


	for (const auto& action : actuatorComponent->getActions())
	{
		if (isPreparation == true)
		{
			if ( (action->isActionMemorized() == true) && (this->memorizedTransitionActionBehavior == SimulationBehavior_t::prepare) )
			{
				action->beginAction();
			}
			else if ( (action->isActionMemorized() == false) && (this->pulseTransitionActionBehavior == SimulationBehavior_t::prepare) )
			{
				action->beginAction();
			}
		}
		else
		{
			if ( (action->isActionMemorized() == true) && (this->memorizedTransitionActionBehavior == SimulationBehavior_t::immediately) )
			{
				action->beginAction();
			}
			else if ( (action->isActionMemorized() == false) && (this->pulseTransitionActionBehavior == SimulationBehavior_t::immediately) )
			{
				action->beginAction();
			}
		}
	}
}

void SimulatedFsm::deactivateStateActions(componentId_t actuatorId)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuatorComponent = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(actuatorId));
	if (actuatorComponent == nullptr) return;


	for (const auto& action : actuatorComponent->getActions())
	{
		action->endAction();
	}
}

void SimulatedFsm::deactivateTransitionActions(componentId_t actuatorId, bool isPreparation)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuatorComponent = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(actuatorId));
	if (actuatorComponent == nullptr) return;


	for (const auto& action : actuatorComponent->getActions())
	{
		if (isPreparation == true)
		{
			if ( (action->isActionMemorized() == true) && (this->memorizedTransitionActionBehavior == SimulationBehavior_t::prepare) )
			{
				action->endAction();
			}
			else if ( (action->isActionMemorized() == false) && (this->pulseTransitionActionBehavior == SimulationBehavior_t::prepare) )
			{
				action->endAction();
			}
		}
		else
		{
			if ( (action->isActionMemorized() == true) && (this->memorizedTransitionActionBehavior == SimulationBehavior_t::immediately) )
			{
				action->endAction();
			}
			else if ( (action->isActionMemorized() == false) && (this->pulseTransitionActionBehavior == SimulationBehavior_t::immediately) )
			{
				action->endAction();
			}
		}
	}
}
