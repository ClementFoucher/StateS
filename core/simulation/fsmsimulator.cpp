/*
 * Copyright © 2014-2015 Clément Foucher
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
#include "signal.h"
#include "fsmtransition.h"
#include "statesexception.h"
#include "truthtable.h"


FsmSimulator::FsmSimulator(shared_ptr<Fsm> machine) :
    MachineSimulator()
{
    this->clock = shared_ptr<Clock>(new Clock());
    connect(this->clock.get(), &Clock::clockEvent, this, &FsmSimulator::clockEventHandler);
    connect(this->clock.get(), &Clock::resetEvent, this, &FsmSimulator::resetEventHandler);

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
    shared_ptr<Fsm> machine = this->machine.lock();

    if (machine != nullptr)
    {
        // First reset signals.
        // Must be done before states as initial state can activate some.
        foreach(shared_ptr<Signal> sig, machine->getVariablesSignals())
        {
            sig->reinitialize();
        }

        if (machine->getInitialState() != nullptr)
        {
            foreach(shared_ptr<FsmState> state, machine->getStates())
            {
                state->setActive(false);
            }

            shared_ptr<FsmState> initialState = machine->getInitialState();
            initialState->setActive(true);

            this->currentState = initialState;
        }
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
        foreach(weak_ptr<Signal> sig, latestTransitionCrossed->getActions())
        {
            shared_ptr<Signal> signal = sig.lock();

            if (signal != nullptr)
            {
                MachineActuatorComponent::action_types temp = latestTransitionCrossed->getActionType(signal);
                if (temp == MachineActuatorComponent::action_types::pulse)
                    signal->resetValue();
            }
        }

        this->latestTransitionCrossed.reset();
    }

    if (currentState != nullptr)
    {
        foreach(weak_ptr<Signal> sig, currentState->getActions())
        {
            shared_ptr<Signal> signal = sig.lock();

            if (signal != nullptr)
            {
                if (currentState->getActionType(signal) == MachineActuatorComponent::action_types::pulse)
                    signal->resetValue();
            }
        }
    }

    //
    // Then look for potential transitions

    foreach(shared_ptr<FsmTransition> transition, currentState->getOutgoingTransitions())
    {
        if (transition->getCondition() != nullptr)
        {
            try
            {
                if (transition->getCondition()->isTrue())
                {
                    potentialTransitions.insert(potentialTransitions.count(), transition);
                }
            }
            catch (const StatesException& e)
            {
                if ( (e.getSourceClass() == "TruthTable") && (e.getEnumValue() == TruthTable::TruthTableErrorEnum::reference_expired) )
                {
                    // Transition condition is incorrect, considered false: nothing to do
                }
                else
                    throw;
            }
        }
        else
        {
            // Empty transition are implicitly set to true
            potentialTransitions.insert(potentialTransitions.count(), transition);
        }
    }

    if (potentialTransitions.count() == 1)
    {
        // One available transition, let's activate it.
        this->activateTransition(potentialTransitions[0]);
    }
    else if (potentialTransitions.count() > 1)
    {
        // If multiple transitions leading to the same state, no way to differentiate them.
        // Ask for wich one to follow.
        // This is just a small instant patch, user should correct his machine.

        this->clock->stop();

        targetStateSelector = unique_ptr<QWidget>(new QWidget());
        QVBoxLayout* choiceWindowLayout = new QVBoxLayout(targetStateSelector.get());

        QLabel* choiceWindowWarningText = new QLabel(tr("Warning! There are multiple active transitions going out the current state!") + "<br />"
                                                     + tr("This means your FSM is wrong by construction. This should be fixed.") + "<br />"
                                                     + tr("For current simulation, just choose the target state in the following list:"));

        choiceWindowLayout->addWidget(choiceWindowWarningText);

        this->signalMapper = new QSignalMapper();

        connect(this->signalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this, &FsmSimulator::targetStateSelectionMadeEventHandler);

        for (int i = 0 ; i < potentialTransitions.count() ; i++)
        {
            QPushButton* button = new QPushButton(potentialTransitions[i]->getTarget()->getName());

            this->signalMapper->setMapping(button, i);

            connect(button, &QPushButton::clicked, this->signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
            connect(button, &QPushButton::clicked, this->targetStateSelector.get(), &QWidget::close);

            choiceWindowLayout->addWidget(button);
        }

        this->targetStateSelector->show();
    }
}

void FsmSimulator::targetStateSelectionMadeEventHandler(int i)
{
    this->targetStateSelector.reset();

    this->signalMapper->deleteLater(); // Can't be deleted now as we are in a call from this object
    this->signalMapper = nullptr;

    this->activateTransition(this->potentialTransitions[i]);
}

void FsmSimulator::forceStateActivation(shared_ptr<FsmState> stateToActivate)
{
    if (! this->currentState.expired())
        this->currentState.lock()->setActive(false);

    this->currentState = stateToActivate;
    stateToActivate->setActive(true);
}


void FsmSimulator::activateTransition(shared_ptr<FsmTransition> transition)
{
    shared_ptr<FsmTransition> l_latestTransition = this->latestTransitionCrossed.lock();

    if (l_latestTransition != nullptr)
        l_latestTransition->deactivateActions();

    transition->activateActions();
    this->latestTransitionCrossed = transition;

    if (! this->currentState.expired())
        this->currentState.lock()->setActive(false);

    shared_ptr<FsmState> newState = transition->getTarget();
    if (newState != nullptr)
    {
        newState->setActive(true);
        this->currentState = newState;
    }

    this->potentialTransitions.clear();
}
