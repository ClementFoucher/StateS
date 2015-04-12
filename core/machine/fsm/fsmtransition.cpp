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
#include "fsmtransition.h"

// Debug
#include <QDebug>

// StateS classes
#include "equation.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmgraphicaltransition.h"


FsmTransition::FsmTransition(shared_ptr<Fsm> parent, shared_ptr<FsmState> source, shared_ptr<FsmState> target, shared_ptr<Signal> condition, FsmGraphicalTransition* graphicalRepresentation) :
    FsmComponent(parent)
{
    this->source = source;
    this->target = target;

    this->setAllowedActionTypes(pulse | set | reset | assign);

    this->setCondition(condition);

    this->setGraphicalRepresentation(graphicalRepresentation);

    // Propagates local events to the more general "configuration changed" event
    connect(this, &FsmTransition::conditionChangedEvent, this, &MachineComponent::componentStaticConfigurationChangedEvent);
}

FsmTransition::~FsmTransition()
{
    delete this->graphicalRepresentation;
}

shared_ptr<FsmState> FsmTransition::getSource() const
{
    return this->source.lock();
}

void FsmTransition::setSource(shared_ptr<FsmState> newSource)
{
    this->source = newSource;
}

FsmGraphicalTransition* FsmTransition::getGraphicalRepresentation() const
{
    return this->graphicalRepresentation;
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
    this->graphicalRepresentation = nullptr;
}

shared_ptr<FsmState> FsmTransition::getTarget() const
{
    return target.lock();
}

void FsmTransition::setTarget(shared_ptr<FsmState> newTarget)
{
    this->target = newTarget;
}

shared_ptr<Signal> FsmTransition::getCondition() const
{
    if (this->condition->getFunction() != Equation::nature::identity)
        return this->condition;
    else
        return this->condition->getOperand(0);
}

void FsmTransition::setCondition(shared_ptr<Signal> signalNewCondition)
{
    if (this->condition != nullptr)
    {
        disconnect(this->condition.get(), &Signal::signalDynamicStateChangedEvent,        this, &MachineComponent::componentDynamicStateChangedEvent);
        disconnect(this->condition.get(), &Signal::signalStaticConfigurationChangedEvent, this, &FsmTransition::conditionChangedEvent);
    }

    shared_ptr<Equation> equationNewCondition = dynamic_pointer_cast<Equation>(signalNewCondition);
    if (equationNewCondition == nullptr)
    {
        equationNewCondition = shared_ptr<Equation>(new Equation(Equation::nature::identity, 1));
        equationNewCondition->setOperand(0, signalNewCondition);
    }

    this->condition = equationNewCondition;

    if (this->condition != nullptr)
    {
        // Propagate events
        connect(this->condition.get(), &Signal::signalDynamicStateChangedEvent,        this, &MachineComponent::componentDynamicStateChangedEvent);
        connect(this->condition.get(), &Signal::signalStaticConfigurationChangedEvent, this, &FsmTransition::conditionChangedEvent);
    }

    emit conditionChangedEvent();
}

void FsmTransition::clearConditionEventHandler(shared_ptr<Signal>)
{
    setCondition(nullptr);
}
