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
#include "equation.h"

// Debug
#include <QDebug>


Equation::Equation(nature function, const QVector<Signal*>& operandList) :
    Equation(function, operandList.count())
{
    for (uint i = 0 ; i < this->allowedOperandCount ; i++)
    {
        this->setOperand(i, operandList[i]);
    }
}

Equation::Equation(nature function, uint operandCount) :
    Signal("<sub>(equation)</sub>")
{
    this->function = function;

    if (this->function == nature::notOp)
    {
        this->allowedOperandCount = 1;
        if (operandCount != 1)
            qDebug() << "(Equation) Error ! Trying to create a NOT equation with size != 1 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 1.";
    }
    else if ( (this->function == nature::equalOp) || (this->function == nature::diffOp))
    {
        this->allowedOperandCount = 2;

        if (operandCount != 2)
        {
            if (function == nature::equalOp)
                qDebug() << "(Equation) Error ! Trying to create a EQUAL equation with size != 2 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 2.";
            else
                qDebug() << "(Equation) Error ! Trying to create a DIFFERENT equation with size != 2 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 2.";
        }
    }
    else
    {
        if (operandCount < 2)
        {
            this->allowedOperandCount = 2;
            qDebug() << "(Equation) Error ! Trying to create an equation with size < 2 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 2.";
        }
        else
            this->allowedOperandCount = operandCount;
    }

    for (uint i = 0 ; i < allowedOperandCount ; i++)
    {
        operands.append(nullptr);
    }
}

Equation::~Equation()
{
    for (uint i = 0 ; i < allowedOperandCount ; i++)
        clearOperand(i);
}

Equation* Equation::clone() const
{
    QVector<Signal*> newEquationOperands;

    for (uint i = 0 ; i < this->allowedOperandCount ; i++)
    {
        // For equation operands (and system constants,) clone them.
        // For signal operands, reference them.

        if (operands[i] != nullptr)
        {
            Signal* currentSignalOperand = this->operands[i];
            Equation* currentEquationOperand = dynamic_cast <Equation*> (currentSignalOperand);

            if (currentEquationOperand != nullptr)
            {
                newEquationOperands.append(currentEquationOperand->clone());
            }
            else
            {
                newEquationOperands.append(currentSignalOperand);
            }
        }
        else
            newEquationOperands.append(nullptr);
    }

    return new Equation(function, newEquationOperands);
}

/**
 * @brief getSize returns the apparent size of the object. 0 means no current size
 * @return
 */
uint Equation::getSize() const
{
    if (this->sizedOperandCount == this->allowedOperandCount)
    {
        if ( (this->function == nature::equalOp) || (this->function == nature::diffOp))
            return 1;
        else
            return currentSignalSize;
    }
    else
        return 0;
}

bool Equation::resize(uint)
{
    // Equation size is dynamic (or is 1 for eq and diff).
    // Can't be resized.
    return false;
}

Equation::nature Equation::getFunction() const
{
    return function;
}

void Equation::setFunction(const nature& newFunction)
{
    if (newFunction == nature::notOp)
    {
        // Delete operands beyond one
        while (allowedOperandCount > 1)
        {
            decreaseOperandCount(true);
        }
    }
    else
    {
        // At least two operands for all other equation types
        if (allowedOperandCount == 1)
        {
            increaseOperandCount(true);
        }
        else if ( (newFunction == nature::equalOp) || (newFunction == nature::diffOp) )
        {
            // Exactly two operands needed
            if (allowedOperandCount > 2)
            {
                // Delete operands beyond two
                while (allowedOperandCount > 2)
                {
                    decreaseOperandCount(true);
                }
            }
        }
    }

    this->function = newFunction;
}

Signal* Equation::getOperand(uint i) const
{
    if (i < this->allowedOperandCount)
        return this->operands[i];
    else
        return nullptr;
}

bool Equation::setOperand(uint i, Signal* newOperand)
{
    // Do not allow placing an operand outside defined range
    if (i >= this->allowedOperandCount)
    {
        return false;
    }

    // Check if operand size matches current size
    if ( (this->sizedOperandCount != 0) && ( this->signalHasSize(newOperand) ) )
    {
        if (this->currentSignalSize != newOperand->getSize())
            return false;
    }

    // It's OK, let's begin replacement

    // Only proceed to set if new operand is not the same as the current one.
    // If it is, just return success
    if (operands[i] == newOperand)
    {
        return true;
    }
    else
    {
        clearOperand(i);

        if (newOperand != nullptr)
        {
            // Assign operand

            Equation* newEquationOperand = dynamic_cast <Equation*> (newOperand);

            if (newEquationOperand != nullptr)
                operands[i] = newEquationOperand->clone();
            else
                operands[i] = newOperand;

            connect(operands[i], &Signal::signalStateChangedEvent,         this, &Signal::signalStateChangedEvent);
            connect(operands[i], &Signal::signalConfigurationChangedEvent, this, &Signal::signalConfigurationChangedEvent);
            connect(operands[i], &Signal::signalDeletedEvent,              this, &Equation::operandDeletedEvent);
            connect(operands[i], &Signal::signalResizedEvent,              this, &Equation::operandResizedEvent);

            if (signalHasSize(operands[i]))
            {
                if (sizedOperandCount == 0)
                {
                    currentSignalSize = operands[i]->getSize();

                    emit signalResizedEvent();
                    emit signalConfigurationChangedEvent();
                }

                sizedOperandCount++;
            }


            actualOperandCount++;
        }

        // In order to actualize display
        emit signalConfigurationChangedEvent();

        return true;
    }
}


void Equation::clearOperand(uint i)
{
    if (operands[i] != nullptr)
    {

        disconnect(operands[i], &Signal::signalStateChangedEvent,         this, &Signal::signalStateChangedEvent);
        disconnect(operands[i], &Signal::signalConfigurationChangedEvent, this, &Signal::signalConfigurationChangedEvent);
        disconnect(operands[i], &Signal::signalDeletedEvent,              this, &Equation::operandDeletedEvent);
        disconnect(operands[i], &Signal::signalResizedEvent,              this, &Equation::operandResizedEvent);


        Equation* equationOperand = dynamic_cast <Equation*> (operands[i]);
        if (equationOperand != nullptr)
        {
            // We hold our own copy of equations: delete it

            if (equationOperand->getSize() != 0)
            {
                sizedOperandCount--;
            }

            delete equationOperand;
        }
        else // Signal
        {
            sizedOperandCount--;
        }

        actualOperandCount--;
        operands[i] = nullptr;

        if (sizedOperandCount == 0)
        {
            this->currentSignalSize = 0;

            emit signalResizedEvent();
        }

        // In order to actualize display
        emit signalConfigurationChangedEvent();
    }
}


QString Equation::getText(bool colored) const
{
    QString text;

    // Inversion oeprator
    if ( (function == nature::notOp)  ||
         (function == nature::nandOp) ||
         (function == nature::norOp)  ||
         (function == nature::xnorOp) )
        text += '/';

    if (this->allowedOperandCount > 1)
        text += "( ";

    for (uint i = 0 ; i < this->allowedOperandCount ; i++)
    {
        if (operands[i] != nullptr)
        {
            text += operands[i]->getText(colored);
        }
        else if (this->currentSignalSize != 0)
        {
            text += "..."; //LogicValue::getValue0(this->currentSignalSize).toString(); // Should handle color
        }

        // Add operator, except for last operand
        if (i < this->allowedOperandCount - 1)
        {
            switch(function)
            {
            case Equation::nature::andOp:
            case Equation::nature::nandOp:
                text += " • ";
                break;
            case Equation::nature::orOp:
            case Equation::nature::norOp:
                text += " + ";
                break;
            case Equation::nature::xorOp:
            case Equation::nature::xnorOp:
                text += " ⊕ ";
                break;
            case Equation::nature::equalOp:
                text += " = ";
                break;
            case Equation::nature::diffOp:
                text += " ≠ ";
                break;
            case Equation::nature::notOp:
                break;
            }
        }
    }

    if (this->allowedOperandCount > 1)
        text += " )";

    return text;
}

LogicValue Equation::getCurrentValue() const
{
    if (this->allowedOperandCount != this->sizedOperandCount)
        return LogicValue::getNullValue();
    else
    {
        if (function == nature::notOp)
        {
            return ! ( operands[0]->getCurrentValue() );
        }
        else if ( (function == nature::equalOp) || (function == nature::diffOp) )
        {
            LogicValue result(1);

            if (function == nature::equalOp)
            {
                result[0] = ((operands[0]->getCurrentValue() == operands[1]->getCurrentValue()));
            }
            else if (function == nature::diffOp)
            {
                result[0] = ((operands[0]->getCurrentValue() != operands[1]->getCurrentValue()));
            }

            return result;
        }
        else
        {
            LogicValue partialResult(currentSignalSize);

            switch (function)
            {
            case nature::andOp:
            case nature::nandOp:

                partialResult = LogicValue(currentSignalSize, true);

                foreach(Signal* operand, operands)
                {
                    partialResult &= operand->getCurrentValue();
                }

                break;

            case nature::orOp:
            case nature::norOp:

                foreach(Signal* operand, operands)
                {
                    partialResult |= operand->getCurrentValue();
                }

                break;

            case nature::xorOp:
            case nature::xnorOp:

                foreach(Signal* operand, operands)
                {
                    partialResult ^= operand->getCurrentValue();
                }

                break;
            case nature::equalOp:
            case nature::diffOp:
            case nature::notOp:
                break;
            }

            if (this->isInverted())
                partialResult = !partialResult;

            return partialResult;
        }
    }
}

bool Equation::isInverted() const
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
    case nature::equalOp:
    case nature::diffOp:
        return false;
        break;
    default:
        qDebug() << "(Logic equation:) Error! Unkown operator type!";
        return false;
    }
}

const QVector<Signal*>& Equation::getOperands() const
{
    return operands;
}

void Equation::operandDeletedEvent(Signal* var)
{
    for (uint i = 0 ; i < this->allowedOperandCount ; i++)
    {
        if (var == operands[i])
        {
            clearOperand(i);
            break;
        }
    }
}

void Equation::operandResizedEvent()
{
    if (this->sizedOperandCount == 1)
    {
        for(uint i = 0 ; i < allowedOperandCount ; i++)
        {
            if ( (operands[i] != nullptr) /*&& (!operands[i]->isSystemConstant())*/ && (operands[i]->getSize() != this->currentSignalSize) )
            {
                this->currentSignalSize = operands[i]->getSize();
                //adjustSize();
            }
        }
    }
    else // Sized operand count > 1
    {
        for(uint i = 0 ; i < allowedOperandCount ; i++)
        {
            if ( (operands[i] != nullptr) && (operands[i]->getSize() != this->currentSignalSize) )
            {
                clearOperand(i);
            }
        }
    }

}

bool Equation::signalHasSize(Signal *sig)
{
    if (sig == nullptr)
        return false;
    else if (sig->getSize() == 0)
        return false;
    else
        return true;
}

uint Equation::getOperandCount() const
{
    return allowedOperandCount;
}

bool Equation::increaseOperandCount(bool force)
{
    if ( ( force ) ||
         ( (function != nature::notOp)   &&
           (function != nature::equalOp) &&
           (function != nature::diffOp)
         )
       )
    {
        this->operands.append(nullptr);
        this->allowedOperandCount++;

        // In order to actualize display
        emit signalConfigurationChangedEvent();

        return true;
    }
    else
        return false;
}

bool Equation::decreaseOperandCount(bool force)
{
    if ( ( force ) ||
         ( (function != nature::notOp)   &&
           (function != nature::equalOp) &&
           (function != nature::diffOp)
         )
       )
    {
        if ( (force) || (this->allowedOperandCount > 2) )
        {
            clearOperand(allowedOperandCount-1);
            operands.removeLast();
            this->allowedOperandCount--;

            // In order to actualize display
            emit signalConfigurationChangedEvent();

            return true;
        }
        else
            return false;
    }
    else
        return false;
}
