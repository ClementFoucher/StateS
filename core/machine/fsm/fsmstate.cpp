/*
 * Copyright © 2014 Clément Foucher
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
#include "fsmstate.h"

// Debug
#include <QDebug>

// StateS classes
#include "fsm.h"
#include "fsmgraphicalstate.h"
#include "fsmtransition.h"
#include "signal.h"


FsmState::FsmState(Fsm* parent) :
    FsmComponent(parent)
{
    // Build name dynamically to ensure uniqueness
    this->setUniqueName();

    this->getOwningFsm()->addState(this);
}

FsmState::FsmState(Fsm* parent, const QString& name) :
    FsmComponent(parent)
{
    // Try given name, but ignore if not unique
    if (!setName(name))
    {
        this->setUniqueName();
        qDebug() << "(Fsm state:) WARNING! Ignore given name " + name + " because already existing in machine. Used name " + this->name + " instead.";
    }

    this->getOwningFsm()->addState(this);
}

FsmState::~FsmState()
{
    clearActions();

    // Delete transitions
    QList<FsmTransition *> allTransitions = inputTransitions;
    allTransitions += outputTransitions;
    // Deal with transitions with both ends connected to state
    QSet<FsmTransition*> uniqueTransitions = allTransitions.toSet();

    qDeleteAll(uniqueTransitions);

    getOwningFsm()->removeState(this);

    // Delete graphics representation
    delete graphicalRepresentation;
}

FsmGraphicalState* FsmState::getGraphicalRepresentation() const
{
    return graphicalRepresentation;
}

void FsmState::setGraphicalRepresentation(FsmGraphicalState* representation)
{
    if (this->graphicalRepresentation == nullptr)
        this->graphicalRepresentation = representation;
    else
        qDebug() << "(Fsm state:) ERROR! Setting graphic representation while already have one. Ignored command.";
}

void FsmState::clearGraphicalRepresentation()
{
    graphicalRepresentation = nullptr;
}

QString FsmState::getName() const
{
    return name;
}

bool FsmState::setName(const QString& newName)
{
    bool nameIsValid = true;

    foreach(FsmState* colleage, getOwningFsm()->getStates())
    {
        if (colleage->getName() == newName)
        {
            nameIsValid = false;
            break;
        }
    }

    if(nameIsValid)
    {
        name = newName;
        emit elementConfigurationChangedEvent();
    }

    return nameIsValid;
}

void FsmState::setUniqueName()
{
    QString baseName = "State #";
    QString currentName;

    uint i = 0;
    bool nameIsValid = false;

    while (!nameIsValid)
    {
        currentName = baseName + QString::number(i);

        nameIsValid = true;
        foreach(FsmState* colleage, this->getOwningFsm()->getStates())
        {
            if (colleage->getName() == currentName)
            {
                nameIsValid = false;
                i++;
                break;
            }
        }
    }
    this->name = currentName;
}

void FsmState::addIncomingTransition(FsmTransition* transition)
{
    this->inputTransitions.append(transition);
}

void FsmState::addOutgoingTransition(FsmTransition* transition)
{
    this->outputTransitions.append(transition);
}

void FsmState::removeIncomingTransition(FsmTransition* transition)
{
    this->inputTransitions.removeAll(transition);
}

void FsmState::removeOutgoingTransition(FsmTransition* transition)
{
    this->outputTransitions.removeAll(transition);
}

const QList<FsmTransition*>& FsmState::getOutgoingTransitions() const
{
    return this->outputTransitions;
}
const QList<FsmTransition*>& FsmState::getIncomingTransitions() const
{
    return this->inputTransitions;
}

bool FsmState::getIsActive() const
{
    return this->isActive;
}

void FsmState::setActive(bool value)
{
    this->isActive = value;

    // On enter state, activate actions
    if (value == true)
        this->activateActions();
    else
    {
        // Disable pulse actions on state leave
        foreach(Signal* sig, this->actions)
        {
            if (actionType[sig] == MachineActuatorComponent::action_types::pulse)
                sig->resetValue();
        }
    }

    emit elementStateChangedEvent();
}

bool FsmState::isInitial() const
{
    return (getOwningFsm()->getInitialState() == this);
}

void FsmState::setInitial()
{
    getOwningFsm()->setInitialState(this);
}

