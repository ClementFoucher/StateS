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
#include "fsm.h"

// Debug
#include <QDebug>

// StateS classes
#include "fsmstate.h"
#include "fsmsimulator.h"


Fsm::Fsm()
{

}

const QList<shared_ptr<FsmState>>& Fsm::getStates() const
{
    return states;
}

shared_ptr<FsmState> Fsm::addState(QString name)
{
    shared_ptr<Fsm> thisAsPointer = dynamic_pointer_cast<Fsm>(this->shared_from_this());
    shared_ptr<FsmState> state(new FsmState(thisAsPointer, getUniqueStateName(name)));
    connect(state.get(), &FsmState::componentStaticConfigurationChangedEvent, this, &Fsm::stateEditedEventHandler);
    connect(state.get(), &FsmState::stateGraphicRepresentationMoved,          this, &Fsm::stateEditedEventHandler);
    states.append(state);

    this->setUnsavedState(true);

    return state;
}

void Fsm::removeState(shared_ptr<FsmState> state)
{
    disconnect(state.get(), &FsmState::componentStaticConfigurationChangedEvent, this, &Fsm::stateEditedEventHandler);
    disconnect(state.get(), &FsmState::stateGraphicRepresentationMoved,          this, &Fsm::stateEditedEventHandler);
    states.removeAll(state);
    this->setUnsavedState(true);
}

bool Fsm::renameState(shared_ptr<FsmState> state, QString newName)
{
    if (state->getName() == newName)
    {
        // Nothing to do
        return true;
    }

    QString cleanedName = newName.trimmed();

    if (state->getName() == cleanedName)
    {
        // Nothing to do, but still force event to reloead text
        state->setName(cleanedName);
        this->setUnsavedState(true);
        return true;
    }

    // By this point, we know the new name is different from current
    QString actualName = getUniqueStateName(cleanedName);

    if (actualName == cleanedName)
    {
        state->setName(actualName);
        this->setUnsavedState(true);
        return true;
    }
    else
    {
        return false;
    }
}

bool Fsm::isEmpty() const
{
    if (Machine::isEmpty() && this->states.isEmpty())
        return true;
    else
        return false;
}

void Fsm::clear()
{
    this->setInitialState(nullptr);
    states.clear();

    Machine::clear();
}

void Fsm::setSimulator(shared_ptr<MachineSimulator> simulator)
{
    // Clear simulator
    if (simulator == nullptr)
    {
        Machine::setSimulator(nullptr);
    }
    else
    {
        // Check if simulator is of right type
        shared_ptr<FsmSimulator> fsmSimulator = dynamic_pointer_cast<FsmSimulator>(simulator);

        if (fsmSimulator != nullptr)
        {
            Machine::setSimulator(fsmSimulator);
        }
    }
}

void Fsm::forceStateActivation(shared_ptr<FsmState> stateToActivate)
{
    shared_ptr<FsmSimulator> simulator = dynamic_pointer_cast<FsmSimulator>(this->getSimulator());

    if (simulator != nullptr)
    {
        simulator->forceStateActivation(stateToActivate);
    }
}

QString Fsm::getUniqueStateName(QString nameProposal)
{
    QString baseName;
    QString currentName;
    uint i;

    if (nameProposal.isEmpty())
    {
        baseName = tr("State");
        currentName = baseName + " #0";
        i = 0;
    }
    else
    {
        baseName = nameProposal;
        currentName = nameProposal;
        i = 1;
    }

    bool nameIsValid = false;
    while (!nameIsValid)
    {
        nameIsValid = true;
        foreach(shared_ptr<FsmState> colleage, this->states)
        {
            if (colleage->getName() == currentName)
            {
                nameIsValid = false;
                i++;
                currentName = baseName + " #" + QString::number(i);
                break;
            }
        }
    }

    return currentName;
}

QList<shared_ptr<FsmTransition>> Fsm::getTransitions() const
{
    QList<shared_ptr<FsmTransition>> transitionList;

    foreach(shared_ptr<FsmState> state, this->states)
    {
        transitionList += state->getOutgoingTransitions();
    }

    return transitionList;
}

void Fsm::setInitialState(const QString& name)
{
    shared_ptr<FsmState> newInitialState = this->getStateByName(name);

    if (newInitialState != nullptr)
    {
        shared_ptr<FsmState> previousInitialState = this->initialState.lock();

        if (newInitialState != previousInitialState)
        {
            this->initialState = newInitialState;

            if (newInitialState != nullptr)
                emit newInitialState->componentStaticConfigurationChangedEvent();

            if (previousInitialState != nullptr)
                emit previousInitialState->componentStaticConfigurationChangedEvent();

            this->setUnsavedState(true);
        }
    }
}

shared_ptr<FsmState> Fsm::getInitialState() const
{
    return this->initialState.lock();
}

void Fsm::stateEditedEventHandler()
{
    this->setUnsavedState(true);
}

shared_ptr<FsmState> Fsm::getStateByName(const QString &name) const
{
    shared_ptr<FsmState> ret = nullptr;

    foreach(shared_ptr<FsmState> ptr, this->states)
    {
        if (ptr->getName() == name)
        {
            ret = ptr;
            break;
        }
    }

    return ret;
}
