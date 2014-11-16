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

#include <QDebug>

#include "fsmtransition.h"

#include "logicequation.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmgraphicaltransition.h"
#include "logicvariable.h"

FsmTransition::FsmTransition(Fsm* parent, FsmState* source, FsmState* target, FsmGraphicalTransition* graphicalRepresentation, LogicVariable* condition) :
    FsmElement(parent)
{

    this->source = source;
    this->target = target;

    if (condition != nullptr)
        this->condition = condition;
    else
        this->condition = LogicVariable::constant1;

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

LogicVariable* FsmTransition::getCondition() const
{
    return condition;
}

void FsmTransition::setCrossed() const
{
    foreach (LogicVariable* var, actions)
    {
        if (actionType[var] == LogicVariable::action_types::pulse)
        {
            var->setCurrentState(true, true);
        }
        else if (actionType[var] == LogicVariable::action_types::set)
        {
            var->setCurrentState(true);
        }
        else if (actionType[var] == LogicVariable::action_types::reset)
        {
            var->setCurrentState(false);
        }
    }
}

void FsmTransition::setCondition(LogicVariable* variable)
{
    if (condition != nullptr)
    {
        disconnect(condition, SIGNAL(renamedEvent()), this, SIGNAL(elementConfigurationChanged()));
        disconnect(condition, SIGNAL(stateChangedEvent()), this, SIGNAL(elementConfigurationChanged()));
        disconnect(condition, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(clearCondition()));
    }


    LogicEquation* temp = dynamic_cast <LogicEquation*> (condition);

    if (temp != nullptr)
        delete condition;

    if (variable != nullptr)
        condition = variable;
    else
        condition = LogicVariable::constant1;

    if (variable != nullptr)
    {
        connect(variable, SIGNAL(renamedEvent()), this, SIGNAL(elementConfigurationChanged()));
        connect(variable, SIGNAL(stateChangedEvent()), this, SIGNAL(elementConfigurationChanged()));
        connect(variable, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(clearCondition()));
    }

    emit elementConfigurationChanged();
}

void FsmTransition::clearCondition()
{
    setCondition(nullptr);
}
