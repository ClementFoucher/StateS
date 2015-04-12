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


// Main constructor
Equation::Equation(nature function, uint operandCount) :
    Signal("<sub>(equation)</sub>"),
    signalOperands(operandCount),
    equationOperands(operandCount)
{
    this->function = function;

    // Local signals to more general ones
    connect(this, &Equation::equationOperandChangedEvent,      this, &Signal::signalStaticConfigurationChangedEvent);
    connect(this, &Equation::equationOperandCountChangedEvent, this, &Signal::signalStaticConfigurationChangedEvent);
    connect(this, &Equation::equationFunctionChangedEvent,     this, &Signal::signalStaticConfigurationChangedEvent);
    // Dynamic value is impacted by those changes
    connect(this, &Equation::equationOperandChangedEvent,      this, &Equation::computeCurrentValue);
    connect(this, &Equation::equationOperandCountChangedEvent, this, &Equation::computeCurrentValue);
    connect(this, &Equation::equationFunctionChangedEvent,     this, &Equation::computeCurrentValue);

    if ( (this->function == nature::notOp) | (this->function == nature::identity) )
    {
        this->allowedOperandCount = 1;
        if (operandCount != 1)
        {
            signalOperands.resize(1);
            equationOperands.resize(1);

            qDebug() << "(Equation) Error ! Trying to create a NOT equation with size != 1 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 1.";
        }
    }
    else if ( (this->function == nature::equalOp) || (this->function == nature::diffOp))
    {
        this->allowedOperandCount = 2;

        if (operandCount != 2)
        {
            signalOperands.resize(2);
            equationOperands.resize(2);

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

            signalOperands.resize(2);
            equationOperands.resize(2);

            qDebug() << "(Equation) Error ! Trying to create an equation with size < 2 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 2.";
        }
        else
            this->allowedOperandCount = operandCount;
    }
}

Equation::Equation(nature function, const QVector<shared_ptr<Signal>>& operandList) :
    Equation(function, operandList.count())
{
    for (int i = 0 ; i < operandList.count() ; i++)
    {
        // Do not compute value as done in a mute way
        this->setOperand(i, operandList[i], true);
    }

    // Causes recomputation
    this->computeCurrentValue();
}

shared_ptr<Equation> Equation::clone() const
{
    return shared_ptr<Equation>(new Equation(this->function, this->getOperands()));
}

/**
 * @brief getSize returns the apparent size of the equation.
 * 0 means current size is not defined => result will be a null signal.
 * @return
 */
uint Equation::getSize() const
{
    return currentValue.getSize();
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
    if ( (newFunction == nature::notOp) | (newFunction == nature::identity) )
    {
        // Delete operands beyond one
        while (allowedOperandCount > 1)
        {
            // Muted: do not causes recomputation
            decreaseOperandCount(true);
        }
    }
    else
    {
        // At least two operands for all other equation types
        if (allowedOperandCount == 1)
        {
            // Muted: do not causes recomputation
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
                    // Muted: do not causes recomputation
                    decreaseOperandCount(true);
                }
            }
        }
    }

    this->function = newFunction;

    // Causes recomputation
    emit equationFunctionChangedEvent();
}

shared_ptr<Signal> Equation::getOperand(uint i) const
{
    if (i < this->allowedOperandCount)
        return this->getOperands()[i];
    else
        return nullptr;
}

/**
 * @brief Equation::setOperand
 * @param i
 * @param newOperand
 * Equation always owns its own Equation operands.
 * All given operands that are equation will be cloned.
 * @return
 */
bool Equation::setOperand(uint i, shared_ptr<Signal> newOperand, bool quiet)
{
    // Do not allow placing an operand outside defined range
    if (i >= this->allowedOperandCount)
    {
        return false;
    }

    // Only proceed to set if new operand is not the same as the current one.
    // If it is, just return success
    if (this->getOperand(i) == newOperand)
    {
        return true;
    }
    else
    {
        // Muted: do not causes recomputation
        clearOperand(i, true);

        if (newOperand != nullptr)
        {
            // Assign operand

            shared_ptr<Equation> newEquationOperand = dynamic_pointer_cast <Equation> (newOperand);
            shared_ptr<Signal> actualNewOperand;

            if (newEquationOperand != nullptr)
            {
                equationOperands[i] = newEquationOperand->clone();
                actualNewOperand = dynamic_pointer_cast<Signal>(equationOperands[i]);
            }
            else
            {
                connect(newOperand.get(), &Signal::signalDeletedEvent, this, &Equation::computeCurrentValue);
                connect(newOperand.get(), &Signal::signalDeletedEvent, this, &Signal::signalStaticConfigurationChangedEvent);
                signalOperands[i] = newOperand;
                actualNewOperand = newOperand;
            }


            // Structural changes are propagated
            connect(actualNewOperand.get(), &Signal::signalStaticConfigurationChangedEvent, this, &Signal::signalStaticConfigurationChangedEvent);
            // Local stuff
            connect(actualNewOperand.get(), &Signal::signalStaticConfigurationChangedEvent, this, &Equation::computeCurrentValue);
            connect(actualNewOperand.get(), &Signal::signalDynamicStateChangedEvent,        this, &Equation::computeCurrentValue);
        }

        if (!quiet)
        {
            // Causes recomputation
            emit equationOperandChangedEvent();
        }

        return true;
    }
}

void Equation::clearOperand(uint i, bool quiet)
{
    shared_ptr<Signal> oldOperand = this->getOperand(i);

    if (oldOperand != nullptr)
    {
        // Signal propagation
        disconnect(oldOperand.get(), &Signal::signalStaticConfigurationChangedEvent, this, &Signal::signalStaticConfigurationChangedEvent);
        // Local stuff
        disconnect(oldOperand.get(), &Signal::signalStaticConfigurationChangedEvent, this, &Equation::computeCurrentValue);
        disconnect(oldOperand.get(), &Signal::signalDynamicStateChangedEvent,        this, &Equation::computeCurrentValue);

        if (! (signalOperands[i].expired()) )
        {
            disconnect(oldOperand.get(), &Signal::signalDeletedEvent, this, &Equation::computeCurrentValue);
            disconnect(oldOperand.get(), &Signal::signalDeletedEvent, this, &Signal::signalStaticConfigurationChangedEvent);
        }

        equationOperands[i].reset();
        signalOperands[i].reset();

        if (!quiet)
        {
            // Causes recomputation
            emit equationOperandChangedEvent();
        }
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
        shared_ptr<Signal> operand = getOperand(i);
        if (operand != nullptr)
        {
            text += operand->getText(colored);
        }
        else
        {
            if (colored)
                text += "<font color=\"red\">...</font>";
            else
                text += "...";
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
            case Equation::nature::identity:
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
    return this->currentValue;
}

Equation::computationFailureCause Equation::getComputationFailureCause() const
{
    return this->failureCause;
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
    case nature::identity:
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

/**
 * @brief Equation::getOperands
 * Warning: this function returns pointers to Equation operands.
 * This must not be used to take ownership of operands.
 * @return
 */
QVector<shared_ptr<Signal>> Equation::getOperands() const
{
    QVector<shared_ptr<Signal>> operands(this->allowedOperandCount);

    for (uint i = 0 ; i < this->allowedOperandCount ; i++)
    {
        if (!signalOperands[i].expired())
            operands[i] = signalOperands[i].lock();
        else if (equationOperands[i] != nullptr)
            operands[i] = equationOperands[i];
    }

    return operands;
}

void Equation::computeCurrentValue()
{
    bool doCompute = true;

    uint operandsSize = 0;

    foreach(shared_ptr<Signal> currentOperand, this->getOperands())
    {
        if (currentOperand == nullptr)
        {
            this->failureCause = computationFailureCause::nullOperand;
            doCompute = false;
            break;
        }
        else if ( currentOperand->getSize() == 0)
        {
            this->failureCause = computationFailureCause::incompleteOperand;
            doCompute = false;
            break;
        }
        else if ( (operandsSize != 0) && (currentOperand->getSize() != operandsSize) )
        {
            this->failureCause = computationFailureCause::sizeMismatch;
            doCompute = false;
            break;
        }
        else if (operandsSize == 0)
            operandsSize = currentOperand->getSize();
    }

    LogicValue previousValue = this->currentValue;

    if (doCompute)
    {
        QVector<shared_ptr<Signal>> operands = this->getOperands();

        if (function == nature::notOp)
        {
            this->currentValue = ! ( operands[0]->getCurrentValue() );
        }
        else if (function == nature::identity)
        {
            this->currentValue = operands[0]->getCurrentValue();
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

            this->currentValue = result;
        }
        else
        {
            LogicValue partialResult(operandsSize);

            switch (function)
            {
            case nature::andOp:
            case nature::nandOp:

                partialResult = LogicValue(operandsSize, true);

                foreach(shared_ptr<Signal> operand, operands)
                {
                    partialResult &= operand->getCurrentValue();
                }

                break;

            case nature::orOp:
            case nature::norOp:

                foreach(shared_ptr<Signal> operand, operands)
                {
                    partialResult |= operand->getCurrentValue();
                }

                break;

            case nature::xorOp:
            case nature::xnorOp:

                foreach(shared_ptr<Signal> operand, operands)
                {
                    partialResult ^= operand->getCurrentValue();
                }

                break;
            case nature::equalOp:
            case nature::diffOp:
            case nature::notOp:
            case nature::identity:
                // Not handled here
                break;
            }

            if (this->isInverted())
                partialResult = !partialResult;

            this->currentValue = partialResult;

        }

        this->failureCause = computationFailureCause::nofail;
    }
    else
    {
        this->currentValue = LogicValue::getNullValue();
    }

    if (previousValue != this->currentValue)
        emit signalDynamicStateChangedEvent();

    if (previousValue.getSize() != this->currentValue.getSize())
        emit signalResizedEvent();
}

bool Equation::signalHasSize(shared_ptr<Signal> sig) const
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

bool Equation::increaseOperandCount(bool quiet)
{
    if ( ( quiet ) ||
         ( (function != nature::notOp)    &&
           (function != nature::identity) &&
           (function != nature::equalOp)  &&
           (function != nature::diffOp)
         )
       )
    {
        this->allowedOperandCount++;

        this->signalOperands.resize(this->allowedOperandCount);
        this->equationOperands.resize(this->allowedOperandCount);

        if (!quiet)
            // Causes recomputation
            emit equationOperandCountChangedEvent();

        return true;
    }
    else
        return false;
}

bool Equation::decreaseOperandCount(bool quiet)
{
    if ( ( quiet ) ||
         ( (function != nature::notOp)    &&
           (function != nature::identity) &&
           (function != nature::equalOp)  &&
           (function != nature::diffOp)
         )
       )
    {
        if ( (quiet) || (this->allowedOperandCount > 2) )
        {
            clearOperand(allowedOperandCount-1);
            this->allowedOperandCount--;

            this->signalOperands.resize(this->allowedOperandCount);
            this->equationOperands.resize(this->allowedOperandCount);

            if (!quiet)
                // Causes recomputation
                emit equationOperandCountChangedEvent();

            return true;
        }
        else
            return false;
    }
    else
        return false;
}
