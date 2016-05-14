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
#include "fsmstate.h"

// Debug
#include <QDebug>

// StateS classes
#include "fsm.h"
#include "fsmgraphicstate.h"
#include "fsmtransition.h"
#include "signal.h"


FsmState::FsmState(shared_ptr<Fsm> parent, const QString& name) :
    FsmComponent(parent)
{
    this->name = name;

    this->setAllowedActionTypes(activeOnState | set | reset | assign);

    // Propagates local events to the more general events
    connect(this, &FsmState::stateRenamedEvent,           this, &MachineComponent::componentStaticConfigurationChangedEvent);
    connect(this, &FsmState::stateLogicStateChangedEvent, this, &MachineComponent::componentDynamicStateChangedEvent);
}

FsmState::~FsmState()
{
    // For each incoming transition, contact owner to explicitly delete it

    this->cleanIncomingTransitionsList(); // Clean first so that all transitions are valid

    QList<weak_ptr<FsmTransition>> transitions = this->inputTransitions;

    foreach(weak_ptr<FsmTransition> transition, transitions)
    {
        shared_ptr<FsmTransition> transitionL = transition.lock();

        if (transitionL != nullptr)
        {
            // Source CAN be nullptr... for auto-transitions!
            // Indeed current object is already out of any shared pointer at deletion time
            if (transitionL->getSource() != nullptr)
                transitionL->getSource()->removeOutgoingTransition(transitionL);
        }
    }

    // Delete graphics representation
    delete graphicRepresentation;
}

FsmGraphicState* FsmState::getGraphicRepresentation() const
{
    return graphicRepresentation;
}

void FsmState::setGraphicRepresentation(FsmGraphicState* representation)
{
    if (this->graphicRepresentation == nullptr)
    {
        this->graphicRepresentation = representation;
        connect(this->graphicRepresentation, &FsmGraphicState::stateMovingEvent, this, &FsmState::stateGraphicRepresentationMoved);
    }
    else
        qDebug() << "(FsmState:) Error! Setting graphic representation while already have one. Ignored command.";
}

void FsmState::clearGraphicRepresentation()
{
    graphicRepresentation = nullptr;
}

void FsmState::cleanIncomingTransitionsList()
{
    QList<weak_ptr<FsmTransition>> newList;

    foreach(weak_ptr<FsmTransition> transition, this->inputTransitions)
    {
        // Keep all transitions except expired ones
        if (!transition.expired())
            newList.append(transition);
    }

    this->inputTransitions = newList;
}

QString FsmState::getName() const
{
    return this->name;
}

void FsmState::setName(const QString& newName)
{
    this->name = newName;
    emit stateRenamedEvent();
}

void FsmState::addOutgoingTransition(shared_ptr<FsmTransition> transition)
{
    this->outputTransitions.append(transition);
    connect(transition.get(), &FsmTransition::componentStaticConfigurationChangedEvent, this, &MachineComponent::componentStaticConfigurationChangedEvent);

    emit componentStaticConfigurationChangedEvent();
}

void FsmState::removeOutgoingTransition(shared_ptr<FsmTransition> transition)
{
    disconnect(transition.get(), &FsmTransition::componentStaticConfigurationChangedEvent, this, &MachineComponent::componentStaticConfigurationChangedEvent);
    this->outputTransitions.removeAll(transition);

    emit componentStaticConfigurationChangedEvent();
}

const QList<shared_ptr<FsmTransition>> FsmState::getOutgoingTransitions() const
{
    return this->outputTransitions;
}

void FsmState::addIncomingTransition(shared_ptr<FsmTransition> transition)
{
    this->cleanIncomingTransitionsList();
    this->inputTransitions.append(transition);
    emit componentStaticConfigurationChangedEvent();
}

void FsmState::removeIncomingTransition(shared_ptr<FsmTransition> transition)
{
    this->cleanIncomingTransitionsList();

    QList<weak_ptr<FsmTransition>> newList;
    foreach(weak_ptr<FsmTransition> oldTransition, this->inputTransitions)
    {
        shared_ptr<FsmTransition> oldTransitionL = oldTransition.lock();

        if (oldTransitionL != nullptr)
        {
            // Keep all transitions except the one being removed
            if (oldTransitionL != transition)
                newList.append(transition);
        }
    }
    this->inputTransitions = newList;
    emit componentStaticConfigurationChangedEvent();
}

const QList<weak_ptr<FsmTransition>> FsmState::getIncomingTransitions()
{
    this->cleanIncomingTransitionsList();
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
        this->deactivateActions();

    emit stateLogicStateChangedEvent();
}

bool FsmState::isInitial() const
{
    shared_ptr<Fsm> owningFsm = this->getOwningFsm();
    if (owningFsm != nullptr)
    {
        return (owningFsm->getInitialState().get() == this);
    }
    else
        return false;
}

void FsmState::setInitial()
{
    shared_ptr<Fsm> owningFsm = this->getOwningFsm();
    if (owningFsm != nullptr)
    {
        owningFsm->setInitialState(this->name);
    }
}

