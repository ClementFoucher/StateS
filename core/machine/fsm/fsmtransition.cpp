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
#include "fsmtransition.h"

// Debug
#include <QDebug>

// StateS classes
#include "equation.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmgraphicaltransition.h"


FsmTransition::FsmTransition(Fsm* parent, FsmState* source, FsmState* target, FsmGraphicalTransition* graphicalRepresentation, Signal* condition) :
    FsmComponent(parent)
{

    this->source = source;
    this->target = target;

    if (condition != nullptr)
        this->condition = condition;

    source->addOutgoingTransition(this);
    target->addIncomingTransition(this);

    this->getOwningFsm()->addTransition(this);

    setGraphicalRepresentation(graphicalRepresentation);
}

FsmTransition::~FsmTransition()
{
    clearCondition();

    source->removeOutgoingTransition(this);
    target->removeIncomingTransition(this);

    this->getOwningFsm()->removeTransition(this);

    delete graphicalRepresentation;
}

FsmState* FsmTransition::getSource() const
{
    return source;
}

void FsmTransition::setSource(FsmState* newSource)
{
    if (source != nullptr)
        source->removeOutgoingTransition(this);

    source = newSource;

    source->addOutgoingTransition(this);
}

FsmGraphicalTransition* FsmTransition::getGraphicalRepresentation() const
{
    return graphicalRepresentation;
}

void FsmTransition::setGraphicalRepresentation(FsmGraphicalTransition* representation)
{
    if (this->graphicalRepresentation == nullptr)
        this->graphicalRepresentation = representation;
    else
        qDebug() << "(Fsm transition:) ERROR! Setting graphic representation while already have one. Ignored command.";
}

void FsmTransition::clearGraphicalRepresentation()
{
    graphicalRepresentation = nullptr;
}

FsmState* FsmTransition::getTarget() const
{
    return target;
}

void FsmTransition::setTarget(FsmState* newTarget)
{
    if (target != nullptr)
        target->removeIncomingTransition(this);

    target = newTarget;

    target->addIncomingTransition(this);
}

Signal* FsmTransition::getCondition() const
{
    return condition;
}

void FsmTransition::setCondition(Signal* signal)
{
    if (condition != nullptr)
    {
        disconnect(signal, &Signal::signalConfigurationChangedEvent, this, &MachineComponent::elementConfigurationChangedEvent);
        disconnect(signal, &Signal::signalStateChangedEvent,               this, &MachineComponent::elementStateChangedEvent);
        disconnect(signal, &Signal::signalDeletedEvent,                    this, &FsmTransition::clearCondition);
    }

    Equation* temp = dynamic_cast <Equation*> (condition);
    if (temp != nullptr)
        delete condition;

    this->condition = signal;

    if (this->condition != nullptr)
    {
        connect(this->condition, &Signal::signalConfigurationChangedEvent, this, &MachineComponent::elementConfigurationChangedEvent);
        connect(this->condition, &Signal::signalStateChangedEvent,               this, &MachineComponent::elementStateChangedEvent);
        connect(this->condition, &Signal::signalDeletedEvent,                    this, &FsmTransition::clearCondition);
    }

    emit elementConfigurationChangedEvent();
}

void FsmTransition::clearCondition()
{
    setCondition(nullptr);
}
