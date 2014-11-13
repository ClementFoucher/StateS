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

#include "logicequation.h"

#include "logicvariable.h"

LogicEquation::LogicEquation(LogicVariable* leftOperand, nature function, LogicVariable* rightOperand)
{
    this->function = function;

    // A logic equation must ALWAYS has its two operands set
    // If one is set to nullptr, it will be affected constant 0.
    // Even for "not", which doesn't have a left operand, left
    // is set to constant 0.
    this->leftOperand  = LogicVariable::constant0;
    this->rightOperand = LogicVariable::constant0;

    setLeftOperand(leftOperand);
    setRightOperand(rightOperand);
}

LogicEquation::~LogicEquation()
{
    // Only delete equations, variables belong to
    // the machine and must not be deleted

    LogicEquation* temp = dynamic_cast <LogicEquation*> (leftOperand);

    if (temp != nullptr)
        delete temp;

    temp = dynamic_cast <LogicEquation*> (rightOperand);

    if (temp != nullptr)
        delete temp;
}

void LogicEquation::childDeletedEvent(LogicVariable* var)
{
    if (var == leftOperand)
    {
        setLeftOperand(nullptr);
    }
    else if (var == rightOperand)
    {
        setRightOperand(nullptr);
    }

    // In order to actualize display
    emit renamedEvent();
}

bool LogicEquation::getCurrentState() const
{
    switch (function)
    {
    case nature::notOp:
        return (! rightOperand->getCurrentState());
        break;
    case nature::andOp:
        return (leftOperand->getCurrentState() && rightOperand->getCurrentState());
        break;
    case nature::orOp:
        return (leftOperand->getCurrentState() || rightOperand->getCurrentState());
        break;
    case nature::xorOp:
        return (leftOperand->getCurrentState() ^ rightOperand->getCurrentState());
        break;
    case nature::nandOp:
        return (! (leftOperand->getCurrentState() && rightOperand->getCurrentState()) );
        break;
    case nature::norOp:
        return (! (leftOperand->getCurrentState() || rightOperand->getCurrentState()) );
        break;
    case nature::xnorOp:
        return (! (leftOperand->getCurrentState() ^ rightOperand->getCurrentState()));
        break;
    }
}

QString LogicEquation::getText() const
{
    QString text = "(";

    // Do not display left operand for "not" operator
    if (function != nature::notOp)
        text += leftOperand->getText();

    switch(function)
    {
    case LogicEquation::nature::notOp:
        text += "not ";
        break;
    case LogicEquation::nature::andOp:
        text += " and ";
        break;
    case LogicEquation::nature::orOp:
        text += " or ";
        break;
    case LogicEquation::nature::xorOp:
        text += " xor ";
        break;
    case LogicEquation::nature::nandOp:
        text += " nand ";
        break;
    case LogicEquation::nature::norOp:
        text += " nor ";
        break;
    case LogicEquation::nature::xnorOp:
        text += " xnor ";
        break;
    }

    text += rightOperand->getText();

    text += ")";

    return text;
}

LogicEquation::nature LogicEquation::getFunction() const
{
    return function;
}

void LogicEquation::setFunction(const nature &value)
{
    function = value;
}

LogicVariable* LogicEquation::getLeftOperand() const
{
    return leftOperand;
}

void LogicEquation::setLeftOperand(LogicVariable* value)
{
    // Ignore set if new value is same as current
    if (leftOperand != value)
    {
        if (leftOperand != LogicVariable::constant0)
        {
            disconnect(leftOperand, SIGNAL(stateChangedEvent()), this, SIGNAL(stateChangedEvent()));
            disconnect(leftOperand, SIGNAL(renamedEvent()), this, SIGNAL(renamedEvent()));
            disconnect(leftOperand, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(childDeletedEvent(LogicVariable*)));
        }

        if (value != nullptr)
        {
            leftOperand = value;

            connect(leftOperand, SIGNAL(stateChangedEvent()), this, SIGNAL(stateChangedEvent()));
            connect(leftOperand, SIGNAL(renamedEvent()), this, SIGNAL(renamedEvent()));
            connect(leftOperand, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(childDeletedEvent(LogicVariable*)));
        }
        else
            leftOperand = LogicVariable::constant0;
    }
}

LogicVariable* LogicEquation::getRightOperand() const
{
    return rightOperand;
}

void LogicEquation::setRightOperand(LogicVariable* value)
{
    // Ignore set if new value is same as current
    if (rightOperand != value)
    {
        if (rightOperand != LogicVariable::constant0)
        {
            disconnect(rightOperand, SIGNAL(stateChangedEvent()), this, SIGNAL(stateChangedEvent()));
            disconnect(rightOperand, SIGNAL(renamedEvent()), this, SIGNAL(renamedEvent()));
            disconnect(rightOperand, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(childDeletedEvent(LogicVariable*)));
        }

        if (value != nullptr)
        {
            rightOperand = value;

            connect(rightOperand, SIGNAL(stateChangedEvent()), this, SIGNAL(stateChangedEvent()));
            connect(rightOperand, SIGNAL(renamedEvent()), this, SIGNAL(renamedEvent()));
            connect(rightOperand, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(childDeletedEvent(LogicVariable*)));
        }
        else
        {
            rightOperand = LogicVariable::constant0;
        }
    }
}

LogicEquation* LogicEquation::clone() const
{
    // For equation children, clone them.
    // For variable children, reference them.

    LogicVariable* leftVariable;
    LogicVariable* rightVariable;

    LogicEquation* leftEquation = dynamic_cast <LogicEquation*> (leftOperand);

    if (leftEquation != nullptr)
        leftVariable = leftEquation->clone();
    else
        leftVariable = leftOperand;

    LogicEquation* rightEquation = dynamic_cast <LogicEquation*> (rightOperand);

    if (rightEquation != nullptr)
        rightVariable = rightEquation->clone();
    else
        rightVariable = rightOperand;

    return new LogicEquation(leftVariable, function, rightVariable);
}
