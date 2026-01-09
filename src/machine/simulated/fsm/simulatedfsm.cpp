/*
 * Copyright © 2014-2026 Clément Foucher
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
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QSignalMapper>
#include <QPushButton>
#include <QDialog>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "simulatedfsmstate.h"
#include "simulatedfsmtransition.h"
#include "simulatedequation.h"
#include "simulatedvariable.h"
#include "simulatedactiononvariable.h"
#include "statesui.h"


void SimulatedFsm::build()
{
	SimulatedMachine::build();

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	this->initialStateId = fsm->getInitialStateId();

	for (const auto& stateId : fsm->getAllStatesIds())
	{
		auto simulatedState = make_shared<SimulatedFsmState>(stateId);
		this->registerSimulatedComponent(stateId, simulatedState);
	}

	for (const auto& transitionId : fsm->getAllTransitionsIds())
	{
		auto simulatedTransition = make_shared<SimulatedFsmTransition>(transitionId);
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
	// Disable currently active state
	auto previousActiveState = this->getSimulatedState(this->activeStateId);
	if (previousActiveState != nullptr)
	{
		previousActiveState->setActive(false);
	}

	// Change currently active state
	this->activeStateId = stateToActivate;

	auto newActiveState = this->getSimulatedState(this->activeStateId);
	if (newActiveState != nullptr)
	{
		// Enable new state
		newActiveState->setActive(true);

		// Enable state actions
		for (const auto& action : newActiveState->getActions())
		{
			if (action == nullptr) continue;


			if ( (action->isActionMemorized() == true) && (this->memorizedStateActionBehavior == SimulationBehavior_t::immediately) )
			{
				action->doAction();
			}
			else if ( (action->isActionMemorized() == false) && (this->continuousStateActionBehavior == SimulationBehavior_t::immediately) )
			{
				action->doAction();
				this->variablesToResetAfterNextStep.append(action->getVariableId());
			}
		}
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

	this->transitionToBeCrossedId = this->potentialTransitionsIds[i];
	this->potentialTransitionsIds.clear();

	emit this->resumeNormalActivitiesEvent();
}

void SimulatedFsm::subMachineReset()
{
	// Clean any remaining internal state
	this->transitionToBeCrossedId = nullId;
	this->variablesToResetBeforeNextStep.clear();
	this->variablesToResetAfterNextStep.clear();
	this->potentialTransitionsIds.clear();

	delete this->targetStateSelector;
	this->targetStateSelector = nullptr;

	delete this->signalMapper;
	this->signalMapper = nullptr;

	// Enable initial state and activate its actions
	this->forceStateActivation(this->initialStateId);
}

void SimulatedFsm::subMachinePrepareStep()
{
	auto currentActiveState = this->getSimulatedState(this->activeStateId);
	if (currentActiveState == nullptr) return;


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
		else // (condition == nullptr)
		{
			// Empty conditions are implicitly true
			candidateTransitions.insert(candidateTransitions.count(), transition->getId());
		}
	}

	if (candidateTransitions.count() == 1)
	{
		// One available transition, it will be crossed.
		this->transitionToBeCrossedId = candidateTransitions[0];
	}
	else if (candidateTransitions.count() > 1)
	{
		// If multiple transitions are crossable, ask for wich one to follow.
		// This is just a small instant workaround, user should correct the machine.

		emit this->emergencyShutDownEvent();

		auto statesUi = static_cast<StatesUi*>(QApplication::activeWindow());
		if (statesUi == nullptr) return;


		this->targetStateSelector = statesUi->getModalDialog();

		auto choiceWindowLayout = new QVBoxLayout(this->targetStateSelector);

		auto choiceWindowWarningText = new QLabel(tr("Warning! There are multiple active transitions going out the current state!") + "<br />"
		                                        + tr("This means your FSM is wrong by construction. This should be fixed.") + "<br />"
		                                        + tr("For current simulation, just choose the target state in the following list:"));

		choiceWindowLayout->addWidget(choiceWindowWarningText);

		this->signalMapper = new QSignalMapper();
		connect(this->signalMapper, &QSignalMapper::mappedInt, this, &SimulatedFsm::targetStateSelectionMadeEventHandler);

		for (int i = 0 ; i < candidateTransitions.count() ; i++)
		{
			auto candidateTransition = this->getSimulatedTransition(candidateTransitions[i]);
			if (candidateTransition == nullptr) continue;


			auto targetState = this->getSimulatedState(candidateTransition->getTargetStateId());
			auto button = new QPushButton(targetState->getName());

			this->signalMapper->setMapping(button, i);

			connect(button, &QPushButton::clicked, this->signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
			connect(button, &QPushButton::clicked, this->targetStateSelector, &QWidget::close);

			choiceWindowLayout->addWidget(button);
		}

		this->potentialTransitionsIds = candidateTransitions;

		this->targetStateSelector->open();
	}
}

void SimulatedFsm::subMachinePrepareActions()
{
	// Reset unmemorized actions
	for (auto& variableId : this->variablesToResetBeforeNextStep)
	{
		auto simulatedVariable = this->getSimulatedVariable(variableId);
		if (simulatedVariable == nullptr) continue;


		simulatedVariable->reinitialize();
	}
	this->variablesToResetBeforeNextStep.clear();

	// Prepare for next actions
	if (this->transitionToBeCrossedId != nullId)
	{
		auto simulatedTransition = this->getSimulatedActuatorComponent(this->transitionToBeCrossedId);
		if (simulatedTransition == nullptr) return;


		for (const auto& action : simulatedTransition->getActions())
		{
			if (action == nullptr) continue;


			if ( (action->isActionMemorized() == true) && (this->memorizedTransitionActionBehavior == SimulationBehavior_t::prepare) )
			{
				action->doAction();
			}
			else if ( (action->isActionMemorized() == false) && (this->pulseTransitionActionBehavior == SimulationBehavior_t::prepare) )
			{
				action->doAction();
				this->variablesToResetBeforeNextStep.append(action->getVariableId());
			}
		}
	}
}

void SimulatedFsm::subMachineDoStep()
{
	auto currentSimulatedState = this->getSimulatedState(this->activeStateId);
	if (currentSimulatedState == nullptr) return;


	// Reset unmemorized actions
	for (auto& variableId : this->variablesToResetAfterNextStep)
	{
		auto simulatedVariable = this->getSimulatedVariable(variableId);
		if (simulatedVariable == nullptr) continue;


		simulatedVariable->reinitialize();
	}
	this->variablesToResetAfterNextStep.clear();

	// Look for postponed actions in current state
	for (const auto& action : currentSimulatedState->getActions())
	{
		if (action == nullptr) continue;


		if ( (action->isActionMemorized() == true) && (this->memorizedStateActionBehavior == SimulationBehavior_t::after) )
		{
			action->doAction();
		}
		else if ( (action->isActionMemorized() == false) && (this->continuousStateActionBehavior == SimulationBehavior_t::after) )
		{
			action->doAction();
			this->variablesToResetAfterNextStep.append(action->getVariableId());
		}
	}

	// Cross transition
	if (this->transitionToBeCrossedId != nullId)
	{
		auto simulatedTransition = this->getSimulatedTransition(this->transitionToBeCrossedId);
		if (simulatedTransition != nullptr)
		{
			// Deactivate previous state
			auto previousActiveState = this->getSimulatedState(this->activeStateId);
			if (previousActiveState != nullptr)
			{
				previousActiveState->setActive(false);
			}

			// Activate transition actions
			for (const auto& action : simulatedTransition->getActions())
			{
				if (action == nullptr) continue;


				if ( (action->isActionMemorized() == true) && (this->memorizedTransitionActionBehavior == SimulationBehavior_t::immediately) )
				{
					action->doAction();
				}
				else if ( (action->isActionMemorized() == false) && (this->pulseTransitionActionBehavior == SimulationBehavior_t::immediately) )
				{
					action->doAction();
					this->variablesToResetAfterNextStep.append(action->getVariableId());
				}
			}

			// Update current state
			this->activeStateId = simulatedTransition->getTargetStateId();
			currentSimulatedState = this->getSimulatedState(this->activeStateId);
			if (currentSimulatedState != nullptr)
			{
				currentSimulatedState->setActive(true);
			}

			emit this->stateChangedEvent();
		}
		this->transitionToBeCrossedId = nullId;
	}

	// Activate state actions
	for (const auto& action : currentSimulatedState->getActions())
	{
		if (action == nullptr) continue;


		if ( (action->isActionMemorized() == true) && (this->memorizedStateActionBehavior == SimulationBehavior_t::immediately) )
		{
			action->doAction();
		}
		else if ( (action->isActionMemorized() == false) && (this->continuousStateActionBehavior == SimulationBehavior_t::immediately) )
		{
			action->doAction();
			auto variableId = action->getVariableId();
			this->variablesToResetAfterNextStep.append(variableId);
			if (this->variablesToResetBeforeNextStep.contains(variableId))
			{
				// If action is handled in current state, remove it from the reset list of transitions
				this->variablesToResetBeforeNextStep.removeOne(variableId);
			}
		}
	}
}
