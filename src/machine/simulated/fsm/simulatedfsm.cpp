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
#include <QDialog>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "simulatedactuatorcomponent.h"
#include "simulatedfsmstate.h"
#include "simulatedfsmtransition.h"
#include "simulatedequation.h"
#include "simulatedactiononvariable.h"


SimulatedFsm::SimulatedFsm() :
	SimulatedMachine()
{

}

void SimulatedFsm::build()
{
	SimulatedMachine::build();

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	this->initialStateId = fsm->getInitialStateId();

	for (const auto& stateId : fsm->getAllStatesIds())
	{
		auto simulatedState = shared_ptr<SimulatedFsmState>(new SimulatedFsmState(stateId));
		this->registerSimulatedComponent(stateId, simulatedState);
	}

	for (const auto& transitionId : fsm->getAllTransitionsIds())
	{
		auto simulatedTransition = shared_ptr<SimulatedFsmTransition>(new SimulatedFsmTransition(transitionId));
		this->registerSimulatedComponent(transitionId, simulatedTransition);
	}
}

shared_ptr<SimulatedFsmState> SimulatedFsm::getSimulatedState(componentId_t componentId) const
{
	return dynamic_pointer_cast<SimulatedFsmState>(this->getSimulatedComponent(componentId));
}

shared_ptr<SimulatedFsmTransition> SimulatedFsm::getSimulatedTransition(componentId_t componentId) const
{
	return dynamic_pointer_cast<SimulatedFsmTransition>(this->getSimulatedComponent(componentId));
}

void SimulatedFsm::forceStateActivation(componentId_t stateToActivate)
{
	if (this->activeStateId != nullId)
	{
		auto previousActiveState = this->getSimulatedState(this->activeStateId);
		if (previousActiveState != nullptr)
		{
			previousActiveState->setActive(false);
			this->deactivateStateActions(this->activeStateId);
		}
	}

	this->activeStateId = stateToActivate;

	auto newActiveState = this->getSimulatedState(this->activeStateId);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
		this->activateStateActions(this->activeStateId, true);
	}
}

componentId_t SimulatedFsm::getInitialStateId() const
{
	return this->initialStateId;
}

componentId_t SimulatedFsm::getActiveStateId() const
{
	return this->activeStateId;
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

void SimulatedFsm::subcomponentReset()
{
	// Clean any remaining internal state
	this->latestTransitionCrossedId = nullId;
	this->transitionToBeCrossedId   = nullId;
	this->potentialTransitionsIds.clear();

	delete this->targetStateSelector;
	this->targetStateSelector = nullptr;

	delete this->signalMapper;
	this->signalMapper = nullptr;

	// Disable currently active state
	if (this->activeStateId != nullId)
	{
		auto previousActiveState = this->getSimulatedState(this->activeStateId);
		if (previousActiveState != nullptr)
		{
			previousActiveState->setActive(false);
		}
	}

	// Enable initial state and activate its actions
	this->activeStateId = this->initialStateId;

	auto newActiveState = this->getSimulatedState(this->activeStateId);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
		this->activateStateActions(this->activeStateId, true);
	}
}

void SimulatedFsm::subcomponentPrepareStep()
{
	this->transitionToBeCrossedId = nullId;

	// Determine if a transition is to be crossed
	auto currentActiveState = this->getSimulatedState(this->activeStateId);
	if (currentActiveState != nullptr)
	{
		//
		// Look for potential transitions
		QMap<uint, componentId_t> candidateTransitions;
		for (const auto& transitionId : currentActiveState->getOutgoingTransitionsIds())
		{
			auto transition = this->getSimulatedTransition(transitionId);
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

			emit this->emergencyShutDownEvent();

			this->targetStateSelector = new QDialog();

			QVBoxLayout* choiceWindowLayout = new QVBoxLayout(this->targetStateSelector);

			QLabel* choiceWindowWarningText = new QLabel(tr("Warning! There are multiple active transitions going out the current state!") + "<br />"
														 + tr("This means your FSM is wrong by construction. This should be fixed.") + "<br />"
														 + tr("For current simulation, just choose the target state in the following list:"));

			choiceWindowLayout->addWidget(choiceWindowWarningText);

			this->signalMapper = new QSignalMapper();
			connect(this->signalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mappedInt), this, &SimulatedFsm::targetStateSelectionMadeEventHandler);

			for (int i = 0 ; i < candidateTransitions.count() ; i++)
			{
				auto candidateTransition = this->getSimulatedTransition(candidateTransitions[i]);
				if (candidateTransition == nullptr) continue;

				auto targetState = this->getSimulatedState(candidateTransition->getTargetStateId());
				QPushButton* button = new QPushButton(targetState->getName());

				this->signalMapper->setMapping(button, i);

				connect(button, &QPushButton::clicked, this->signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
				connect(button, &QPushButton::clicked, this->targetStateSelector, &QWidget::close);

				choiceWindowLayout->addWidget(button);
			}

			this->potentialTransitionsIds = candidateTransitions;

			this->targetStateSelector->setModal(true);
			this->targetStateSelector->show();
		}
	}
}

void SimulatedFsm::subcomponentPrepareActions()
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

void SimulatedFsm::subcomponentDoStep()
{
	// First reset pulse variables
	if (this->latestTransitionCrossedId != nullId)
	{
		this->deactivateTransitionActions(this->latestTransitionCrossedId, false);
		this->latestTransitionCrossedId = nullId;
	}

	// Look for memorized actions in current state
	this->activateStateActions(this->activeStateId, false);

	// Then determine if a transition is to be crossed
	if (this->transitionToBeCrossedId != nullId)
	{
		this->crossTransition(this->transitionToBeCrossedId);
	}
}

void SimulatedFsm::crossTransition(componentId_t transitionId)
{
	auto previousActiveState = this->getSimulatedState(this->activeStateId);
	if (previousActiveState != nullptr)
	{
		previousActiveState->setActive(false);
		this->deactivateStateActions(this->activeStateId);
	}

	this->latestTransitionCrossedId = transitionId;

	auto simulatedTransition = this->getSimulatedTransition(transitionId);
	if (simulatedTransition != nullptr)
	{
		this->activateTransitionActions(transitionId, false);
		this->activeStateId = simulatedTransition->getTargetStateId();
	}

	auto newActiveState = this->getSimulatedState(this->activeStateId);
	if (newActiveState != nullptr)
	{
		newActiveState->setActive(true);
		this->activateStateActions(this->activeStateId, true);
	}

	emit stateChangedEvent();
}

void SimulatedFsm::activateStateActions(componentId_t actuatorId, bool isFirstActivation)
{
	auto simulatedActuatorComponent = this->getSimulatedActuatorComponent(actuatorId);
	if (simulatedActuatorComponent == nullptr) return;


	for (const auto& action : simulatedActuatorComponent->getActions())
	{
		if (action == nullptr) continue;


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
	auto simulatedActuatorComponent = this->getSimulatedActuatorComponent(actuatorId);
	if (simulatedActuatorComponent == nullptr) return;


	for (const auto& action : simulatedActuatorComponent->getActions())
	{
		if (action == nullptr) continue;


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
	auto simulatedActuatorComponent = this->getSimulatedActuatorComponent(actuatorId);
	if (simulatedActuatorComponent == nullptr) return;


	for (const auto& action : simulatedActuatorComponent->getActions())
	{
		if (action == nullptr) continue;


		action->endAction();
	}
}

void SimulatedFsm::deactivateTransitionActions(componentId_t actuatorId, bool isPreparation)
{
	auto actuatorComponent = this->getSimulatedActuatorComponent(actuatorId);
	if (actuatorComponent == nullptr) return;


	for (const auto& action : actuatorComponent->getActions())
	{
		if (action == nullptr) continue;


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
