/*
 * Copyright © 2014-2020 Clément Foucher
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
#include "clock.h"
#include "fsm.h"
#include "fsmstate.h"
#include "StateS_signal.h"
#include "output.h"
#include "fsmtransition.h"
#include "statesexception.h"
#include "truthtable.h"


FsmSimulator::FsmSimulator(shared_ptr<Fsm> machine) :
    MachineSimulator()
{
	this->clock = shared_ptr<Clock>(new Clock());
	connect(this->clock.get(), &Clock::clockEvent,      this, &FsmSimulator::clockEventHandler);
	connect(this->clock.get(), &Clock::resetLogicEvent, this, &FsmSimulator::resetEventHandler);

	this->machine = machine;

	this->reset();
}

void FsmSimulator::enableOutputDelay(bool enable)
{
	emit outputDelayChangedEvent(enable);
}

shared_ptr<Clock> FsmSimulator::getClock() const
{
	return this->clock;
}

void FsmSimulator::reset()
{
	this->clock->reset();
}

void FsmSimulator::doStep()
{
	this->clock->nextStep();
}

void FsmSimulator::start(uint period)
{
	this->clock->start(period);
}

void FsmSimulator::suspend()
{
	this->clock->stop();
}

void FsmSimulator::resetEventHandler()
{
	shared_ptr<Fsm> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		this->latestTransitionCrossed.reset();

		// Disable any active state
		foreach(shared_ptr<FsmState> state, l_machine->getStates())
		{
			state->setActive(false);
		}

		// Reset inputs and variables to their initial value
		foreach(shared_ptr<Signal> sig, l_machine->getReadableVariableSignals())
		{
			sig->reinitialize();
		}

		// Then compute outputs: first resel all of them...
		foreach(shared_ptr<Output> sig, l_machine->getOutputs())
		{
			sig->resetValue();
		}

		// ... then enable initial state which activate its actions
		shared_ptr<FsmState> initialState = l_machine->getInitialState();
		initialState->setActive(true);
		this->currentState = initialState;
	}
}

void FsmSimulator::clockEventHandler()
{
	shared_ptr<FsmState> currentState = this->currentState.lock();
	shared_ptr<FsmTransition> latestTransitionCrossed = this->latestTransitionCrossed.lock();

	//
	// First reset pulse signals
	if (latestTransitionCrossed != nullptr)
	{
		latestTransitionCrossed->deactivateActions();
		this->latestTransitionCrossed.reset();
	}

	//
	// Then determine if a transition is to be crossed
	if (currentState != nullptr)
	{
		//
		// Look for potential transitions
		QMap<uint, shared_ptr<FsmTransition>> candidateTransitions;
		foreach(shared_ptr<FsmTransition> transition, currentState->getOutgoingTransitions())
		{
			if (transition->getCondition() != nullptr)
			{
				try
				{
					if (transition->getCondition()->isTrue())
					{
						candidateTransitions.insert(candidateTransitions.count(), transition);
					}
				}
				catch (const StatesException& e)
				{
					if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::signal_is_not_bool) )
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
				candidateTransitions.insert(candidateTransitions.count(), transition);
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

			connect(this->signalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mappedInt), this, &FsmSimulator::targetStateSelectionMadeEventHandler);

			for (int i = 0 ; i < candidateTransitions.count() ; i++)
			{
				QPushButton* button = new QPushButton(candidateTransitions[i]->getTarget()->getName());

				this->signalMapper->setMapping(button, i);

				connect(button, &QPushButton::clicked, this->signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
				connect(button, &QPushButton::clicked, this->targetStateSelector, &QWidget::close);

				choiceWindowLayout->addWidget(button);
			}

			this->potentialTransitions = candidateTransitions;
			this->targetStateSelector->show();
		}
	}
}

void FsmSimulator::targetStateSelectionMadeEventHandler(int i)
{
	delete this->targetStateSelector;
	this->targetStateSelector = nullptr;

	this->signalMapper->deleteLater(); // Can't be deleted now as we are in a call from this object
	this->signalMapper = nullptr;

	this->activateTransition(this->potentialTransitions[i]);

	this->potentialTransitions.clear();
}

void FsmSimulator::forceStateActivation(shared_ptr<FsmState> stateToActivate)
{
	shared_ptr<FsmState> l_state = this->currentState.lock();
	if (l_state != nullptr)
	{
		l_state->setActive(false);
	}

	this->currentState = stateToActivate;
	stateToActivate->setActive(true);
}


void FsmSimulator::activateTransition(shared_ptr<FsmTransition> transition)
{
	shared_ptr<FsmState> l_state = this->currentState.lock();
	if (l_state != nullptr)
	{
		l_state->setActive(false);
	}

	transition->activateActions();
	this->latestTransitionCrossed = transition;

	shared_ptr<FsmState> newState = transition->getTarget();
	if (newState != nullptr)
	{
		newState->setActive(true);
		this->currentState = newState;
	}
}
