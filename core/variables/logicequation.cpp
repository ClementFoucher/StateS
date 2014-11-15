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

#include <QDebug>

#include "logicvariable.h"

LogicEquation::LogicEquation(uint size, nature function, LogicVariable* operand1, LogicVariable* operand2)
{
    this->size = size;
    this->function = function;

    // A logic equation must ALWAYS have all its operands set
    // If one is set to nullptr, it will be affected constant 0.
    for(int i = 0 ; i < size ; i ++)
        this->operands[i] = LogicVariable::constant0;

    if (operand1 != nullptr)
        setOperand(0, operand1);

    if (operand2 != nullptr)
        setOperand(1, operand2);
}

LogicEquation::LogicEquation(nature function, const QMap<int, LogicVariable *> &operandList) :
    LogicEquation(operandList.size(), function)
{
    for (int i = 0 ; i < this->size ; i++)
    {
        if (operandList[i] != nullptr)
            setOperand(i, operandList[i]);
    }
}

LogicEquation::~LogicEquation()
{
    // Only delete equations, variables belong to
    // the machine and must not be deleted
    foreach (LogicVariable* var, this->operands)
    {
        LogicEquation* temp = dynamic_cast <LogicEquation*> (var);

        if (temp != nullptr)
            delete temp;
    }
}

void LogicEquation::childDeletedEvent(LogicVariable* var)
{
    for (int i = 0 ; i < this->size ; i++)
    {
        if (var == operands[i])
            setOperand(i, nullptr);
    }

    // In order to actualize display
    emit renamedEvent();
}

const QMap<int, LogicVariable*>& LogicEquation::getOperands() const
{
    return operands;
}

uint LogicEquation::getSize() const
{
    return size;
}

bool LogicEquation::getCurrentState() const
{
    bool computedValue = false;

    if (this->function == nature::notOp)
    {
        computedValue = !( operands[0]->getCurrentState() );
    }
    else
    {
        switch (function)
        {
        case nature::andOp:
        case nature::nandOp:
            computedValue = true;
            foreach(LogicVariable* var, operands)
            {
                if (var->getCurrentState() == false)
                {
                    computedValue = false;
                    break;
                }
            }
            break;

        case nature::orOp:
        case nature::norOp:
            computedValue = false;
            foreach(LogicVariable* var, operands)
            {
                if (var->getCurrentState() == true)
                {
                    computedValue = true;
                    break;
                }
            }
            break;

        case nature::xorOp:
        case nature::xnorOp:
            uint count = 0;
            foreach(LogicVariable* var, operands)
            {
                if (var->getCurrentState() == true)
                {
                    count++;
                }
            }
            if (count % 2 == 1)
                computedValue = true;
            else
                computedValue = false;
            break;
        }

        if (this->isInverted())
            computedValue = ! computedValue;
    }

    return computedValue;
}

bool LogicEquation::isInverted() const
{
    switch (function)
    {
    case nature::notOp:
    case nature::nandOp:
    case nature::norOp:
    case nature::xnorOp:
        return true;
        break;
    case nature::andOp:
    case nature::orOp:
    case nature::xorOp:
        return false;
        break;
    default:
        qDebug() << "(Logic equation:) Error! Unkown operator type!";
        return false;
    }
}

QString LogicEquation::getText() const
{
    QString text;

    if (this->size > 1)
        text = "(";

    // Inversion oeprator
    if ( (function == nature::notOp)  ||
         (function == nature::nandOp) ||
         (function == nature::norOp)  ||
         (function == nature::xnorOp) )
        text += '/';

    for (int i = 0 ; i < this->size ; i++)
    {
        text += operands[i]->getText();

        // Add operator, except for last operand
        if (i < this->size - 1)
        {
            switch(function)
            {
            case LogicEquation::nature::andOp:
            case LogicEquation::nature::nandOp:
                text += " • ";
                break;
            case LogicEquation::nature::orOp:
            case LogicEquation::nature::norOp:
                text += " + ";
                break;
            case LogicEquation::nature::xorOp:
            case LogicEquation::nature::xnorOp:
                text += " ⊕ ";
                break;
            default:
                break;
            }
        }
    }

    if (this->size > 1)
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

LogicVariable* LogicEquation::getOperand(uint i) const
{
    if (i < this->size)
        return this->operands[i];
    else
        return nullptr;
}

void LogicEquation::setOperand(uint i, LogicVariable* newOperand)
{
    if (i < this->size)
    {
        // Ignore set if new value is same as current
        if (operands[i] != newOperand)
        {
            LogicVariable* oldOperand = operands[i];

            if (oldOperand != LogicVariable::constant0)
            {
                disconnect(oldOperand, SIGNAL(stateChangedEvent()), this, SIGNAL(stateChangedEvent()));
                disconnect(oldOperand, SIGNAL(renamedEvent()), this, SIGNAL(renamedEvent()));
                disconnect(oldOperand, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(childDeletedEvent(LogicVariable*)));
            }

            if (newOperand != nullptr)
            {
                operands[i] = newOperand;

                connect(newOperand, SIGNAL(stateChangedEvent()), this, SIGNAL(stateChangedEvent()));
                connect(newOperand, SIGNAL(renamedEvent()), this, SIGNAL(renamedEvent()));
                connect(newOperand, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(childDeletedEvent(LogicVariable*)));
            }
            else
                operands[i] = LogicVariable::constant0;
        }
    }
}

void LogicEquation::increaseOperandCount()
{
    this->operands[this->size] = LogicVariable::constant0;
    this->size++;
}

void LogicEquation::decreaseOperandCount()
{
    if (dynamic_cast<LogicEquation*>(this->operands[this->size-1]) != nullptr)
    {
        delete this->operands[this->size-1];
    }

    this->operands.remove(this->size-1);
    this->size--;
}

LogicEquation* LogicEquation::clone() const
{
    QMap<int, LogicVariable*> variables;

    for (int i = 0 ; i < this->size ; i++)
    {
        if (this->operands[i] != LogicVariable::constant0)
        {
            // For equation children, clone them.
            // For variable children, reference them.

            LogicEquation* complexOperand = dynamic_cast<LogicEquation*>(this->operands[i]);

            if (complexOperand != nullptr)
                variables[i] = complexOperand->clone();
            else
                variables[i] = this->operands[i];
        }
        else
            variables[i] = nullptr;
    }

    return new LogicEquation(function, variables);
}
