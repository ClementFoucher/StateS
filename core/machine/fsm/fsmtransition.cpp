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
#include "fsmgraphictransition.h"


FsmTransition::FsmTransition(shared_ptr<Fsm> parent, shared_ptr<FsmState> source, shared_ptr<FsmState> target, shared_ptr<Signal> condition, FsmGraphicTransition* graphicRepresentation) :
    FsmComponent(parent)
{
    this->source = source;
    this->target = target;

    this->setAllowedActionTypes(pulse | set | reset | assign);

    this->setCondition(condition);

    if (graphicRepresentation != nullptr)
        this->setGraphicRepresentation(graphicRepresentation);

    // Propagates local events to the more general "configuration changed" event
    connect(this, &FsmTransition::conditionChangedEvent, this, &MachineComponent::componentStaticConfigurationChangedEvent);
}

FsmTransition::~FsmTransition()
{
    delete this->graphicRepresentation;
}

shared_ptr<FsmState> FsmTransition::getSource() const
{
    return this->source.lock();
}

void FsmTransition::setSource(shared_ptr<FsmState> newSource)
{
    this->source = newSource;
}

FsmGraphicTransition* FsmTransition::getGraphicRepresentation() const
{
    return this->graphicRepresentation;
}

void FsmTransition::setGraphicRepresentation(FsmGraphicTransition* representation)
{
    if (this->graphicRepresentation == nullptr)
    {
        this->graphicRepresentation = representation;
        connect(representation, &FsmGraphicTransition::graphicTransitionEdited, this, &FsmTransition::graphicTransitionEditedEvent);
    }
    else
        qDebug() << "(FsmTransition:) Error! Setting graphic representation while already have one. Ignored command.";
}

void FsmTransition::clearGraphicRepresentation()
{
    if (this->graphicRepresentation != nullptr)
    {
        disconnect(this->graphicRepresentation, &FsmGraphicTransition::graphicTransitionEdited, this, &FsmTransition::graphicTransitionEditedEvent);
    }
    this->graphicRepresentation = nullptr;
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
        return this->condition->getOperand(0); // Throws StatesException - Identity op always has operand 0, even if nullptr - ignored
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
        QVector<shared_ptr<Signal>> operand;
        operand.append(signalNewCondition);
        equationNewCondition = shared_ptr<Equation>(new Equation(Equation::nature::identity, operand));
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

void FsmTransition::clearCondition()
{
    setCondition(nullptr);
}
