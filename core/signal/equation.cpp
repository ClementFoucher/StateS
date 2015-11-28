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
Equation::Equation(nature function, uint operandCount, int param1, int param2) :
    Signal("<sub>(equation)</sub>"),
    signalOperands(operandCount),
    equationOperands(operandCount)
{
    this->function = function;

    switch(this->function)
    {
    case nature::constant:
        // Constant will be a faked 1 operand, but actually 0
    case nature::notOp:
    case nature::identity:
    case nature::extractOp:
        this->allowedOperandCount = 1;
        this->param1 = param1;
        this->param2 = param2;
        if (operandCount != 1)
        {
            signalOperands.resize(1);
            equationOperands.resize(1);

            qDebug() << "(Equation) Error ! Trying to create a fixed size 1 equation with size != 1 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 1.";
        }
        break;
    case nature::equalOp:
    case nature::diffOp:
        this->allowedOperandCount = 2;

        if (operandCount != 2)
        {
            signalOperands.resize(2);
            equationOperands.resize(2);

            qDebug() << "(Equation) Error ! Trying to create a fixed size 2 equation with size != 2 (requested size is " << QString::number(operandCount) << "). Requested size value ignored and set to 2.";
        }
        break;
    case nature::andOp:
    case nature::orOp:
    case nature::xorOp:
    case nature::nandOp:
    case nature::norOp:
    case nature::xnorOp:
    case nature::concatOp:
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

Equation::Equation(nature function, const QVector<shared_ptr<Signal>>& operandList, int param1, int param2) :
    Equation(function, operandList.count(), param1, param2)
{
    for (int i = 0 ; i < operandList.count() ; i++)
    {
        // Do not compute value as done in a mute way
        this->setOperand(i, operandList[i], true);
    }

    // Causes recomputation
    this->computeCurrentValue();

    this->notYetConstructed = false;
}

shared_ptr<Equation> Equation::clone() const
{
    shared_ptr<Equation> eq = shared_ptr<Equation>(new Equation(this->function, this->getOperands(), this->param1, this->param2));

    if (this->function == nature::constant)
        eq->setCurrentValue(this->currentValue);

    return eq;
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

void Equation::setFunction(const nature& newFunction, int param1, int param2)
{
    switch(this->function)
    {
    case nature::notOp:
    case nature::identity:
    case nature::extractOp:
        // Delete operands beyond one
        while (allowedOperandCount > 1)
        {
            // Muted: do not causes recomputation
            decreaseOperandCountInternal();
        }
        break;
    case nature::equalOp:
    case nature::diffOp:
        // Exactly two operands needed

        // Delete operands beyond two
        while (allowedOperandCount > 2)
        {
            // Muted: do not causes recomputation
            decreaseOperandCountInternal();
        }

        // Add operand if not enough
        if (allowedOperandCount == 1)
        {
            // Muted: do not causes recomputation
            increaseOperandCountInternal();
        }

        break;
    case nature::andOp:
    case nature::orOp:
    case nature::xorOp:
    case nature::nandOp:
    case nature::norOp:
    case nature::xnorOp:
    case nature::concatOp:
        // At least two operands for all other equation types
        if (allowedOperandCount == 1)
        {
            // Muted: do not causes recomputation
            increaseOperandCountInternal();
        }
        break;
    case nature::constant:
        // Ignored
        break;
    }

    this->function = newFunction;

    if (this->function == nature::extractOp)
    {
        this->param1 = param1;
        this->param2 = param2;
    }
    else
    {
        this->param1 = -1;
        this->param2 = -1;
    }

    emit signalStaticConfigurationChangedEvent();

    this->computeCurrentValue();
}

void Equation::setParameters(int param1, int param2)
{
    this->param1 = param1;
    this->param2 = param2;

    emit signalStaticConfigurationChangedEvent();

    this->computeCurrentValue();
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
            emit signalStaticConfigurationChangedEvent();

            this->computeCurrentValue();
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

        if (oldOperand != nullptr)
        {
            disconnect(oldOperand.get(), &Signal::signalDeletedEvent, this, &Equation::computeCurrentValue);
            disconnect(oldOperand.get(), &Signal::signalDeletedEvent, this, &Signal::signalStaticConfigurationChangedEvent);
        }

        equationOperands[i].reset();
        signalOperands[i].reset();

        if (!quiet)
        {
            emit signalStaticConfigurationChangedEvent();

            this->computeCurrentValue();
        }
    }
}


QString Equation::getText(bool activeColored) const
{
    QString errorTextBegin;
    QString errorTextEnd;

    if (this->getSize() == 0)
    {
        errorTextBegin = "<font color=\"red\">";
        errorTextEnd   = "</font>";
    }
    QString text;


    text += errorTextBegin;

    if (this->function == nature::constant)
    {
        text += this->currentValue.toString();
    }
    else
    {
        // Inversion oeprator
        if (this->isInverted())
            text += '/';

        if (this->allowedOperandCount > 1)
            text += "( ";

        for (uint i = 0 ; i < this->allowedOperandCount ; i++)
        {
            shared_ptr<Signal> operand = getOperand(i);
            if (operand != nullptr)
            {
                text += operand->getText(activeColored);
            }
            else
            {
                text += "…";
            }

            // Add operator, except for last operand
            if (i < this->allowedOperandCount - 1)
            {
                switch(function)
                {
                case nature::andOp:
                case nature::nandOp:
                    text += " • ";
                    break;
                case nature::orOp:
                case nature::norOp:
                    text += " + ";
                    break;
                case nature::xorOp:
                case nature::xnorOp:
                    text += " ⊕ ";
                    break;
                case nature::equalOp:
                    text += " = ";
                    break;
                case nature::diffOp:
                    text += " ≠ ";
                    break;
                case nature::concatOp:
                    text += " : ";
                    break;
                case nature::notOp:
                case nature::identity:
                case nature::extractOp:
                case nature::constant:
                    break;
                }
            }
        }

        if (this->function == nature::extractOp)
        {
            text += "[";

            if (this->param1 != -1)
                text += QString::number(this->param1);
            else
                text += "…";

            if (this->param2 != -1)
            {
                text += ".." + QString::number(this->param2);
            }

            text += "]";
        }

        if (this->allowedOperandCount > 1)
            text += " )";
    }

    text += errorTextEnd;

    return text;
}

LogicValue Equation::getCurrentValue() const
{
    return this->currentValue;
}

bool Equation::setCurrentValue(const LogicValue& value)
{
    if (this->function == nature::constant)
    {
        this->currentValue = value;

        emit Signal::signalStaticConfigurationChangedEvent();
        return true;
    }
    else
        return false;
}

Equation::computationFailureCause Equation::getComputationFailureCause() const
{
    return this->failureCause;
}

bool Equation::isInverted() const
{
    bool result = false;

    switch (function)
    {
    case nature::notOp:
    case nature::nandOp:
    case nature::norOp:
    case nature::xnorOp:
        result = true;
        break;
    case nature::identity:
    case nature::andOp:
    case nature::orOp:
    case nature::xorOp:
    case nature::equalOp:
    case nature::diffOp:
    case nature::extractOp:
    case nature::concatOp:
    case nature::constant:
        break;
    }

    return result;
}

int Equation::getParam1() const
{
    return this->param1;
}

int Equation::getParam2() const
{
    return this->param2;
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
    if (this->function == nature::constant)
        return;

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
            if (this->function != nature::concatOp)
            {
                this->failureCause = computationFailureCause::sizeMismatch;
                doCompute = false;
                break;
            }
        }
        else if (operandsSize == 0)
            operandsSize = currentOperand->getSize();
    }

    LogicValue previousValue = this->currentValue;

    if (doCompute)
    {
        QVector<shared_ptr<Signal>> operands = this->getOperands();
        this->failureCause = computationFailureCause::nofail;

        switch (this->function)
        {
        case nature::notOp:
            this->currentValue = ! ( operands[0]->getCurrentValue() );
            break;
        case nature::identity:
            this->currentValue = operands[0]->getCurrentValue();
            break;
        case nature::equalOp:
        case nature::diffOp:
        {
            LogicValue oneBitResult(1);
            if (function == nature::equalOp)
            {
                oneBitResult[0] = ((operands[0]->getCurrentValue() == operands[1]->getCurrentValue()));
            }
            else if (function == nature::diffOp)
            {
                oneBitResult[0] = ((operands[0]->getCurrentValue() != operands[1]->getCurrentValue()));
            }

            this->currentValue = oneBitResult;
        }
            break;
        case nature::extractOp:
            if (this->param1 != -1)
            {
                if (this->param2 != -1)
                {
                    if (this->param1 >= this->param2)
                    {
                        int range = this->param1 - this->param2 + 1;
                        LogicValue subVector(range);
                        LogicValue originalValue = operands[0]->getCurrentValue();

                        for (int i = 0 ; i < range ; i++)
                        {
                            subVector[i] = originalValue[this->param2 + i];
                        }

                        this->currentValue = subVector;

                    }
                    else
                    {
                        this->currentValue = LogicValue::getNullValue();
                        this->failureCause = computationFailureCause::incorrectParameter;
                    }
                }
                else
                {
                    if ((uint)param1 < operands[0]->getSize())
                    {
                        LogicValue result(1);
                        result[0] = operands[0]->getCurrentValue()[param1];
                        this->currentValue = result;
                    }
                    else
                    {
                        param1 = -1;
                        this->currentValue = LogicValue::getNullValue();
                        this->failureCause = computationFailureCause::incorrectParameter;
                    }
                }
            }
            else
            {
                this->currentValue = LogicValue::getNullValue();
                this->failureCause = computationFailureCause::missingParameter;
            }
            break;
        case nature::concatOp:
        {
            int sizeCount = 0;
            foreach (shared_ptr<Signal> currentOperand, this->getOperands())
            {
                sizeCount += currentOperand->getSize();
            }

            LogicValue concatVector(sizeCount);

            int currentBit = sizeCount - 1;
            foreach (shared_ptr<Signal> currentOperand, this->getOperands())
            {
                for (int i = currentOperand->getSize()-1 ; i >= 0 ; i--)
                {
                    concatVector[currentBit] = currentOperand->getCurrentValue()[i];
                    currentBit--;
                }
            }

            this->currentValue = concatVector;
        }
            break;
        case nature::andOp:
        case nature::nandOp:
        {
            LogicValue partialResult(operandsSize, true);
            foreach(shared_ptr<Signal> operand, operands)
            {
                partialResult &= operand->getCurrentValue();
            }

            if (this->isInverted())
                partialResult = !partialResult;

            this->currentValue = partialResult;
        }
            break;

        case nature::orOp:
        case nature::norOp:
        {
            LogicValue partialResult(operandsSize);
            foreach(shared_ptr<Signal> operand, operands)
            {
                partialResult |= operand->getCurrentValue();
            }

            if (this->isInverted())
                partialResult = !partialResult;

            this->currentValue = partialResult;
        }
            break;

        case nature::xorOp:
        case nature::xnorOp:
        {
            LogicValue partialResult(operandsSize);
            foreach(shared_ptr<Signal> operand, operands)
            {
                partialResult ^= operand->getCurrentValue();
            }

            if (this->isInverted())
                partialResult = !partialResult;

            this->currentValue = partialResult;
        }
            break;
        case nature::constant:
            // Nothing to do
            break;
        }
    }
    else
    {
        this->currentValue = LogicValue::getNullValue();
    }

    emit signalDynamicStateChangedEvent();

    if ( (!this->notYetConstructed) && (previousValue.getSize() != this->currentValue.getSize()) )
    {
        emit signalResizedEvent(this->shared_from_this());
    }

}

bool Equation::signalHasSize(shared_ptr<Signal> sig)
{
    if (sig == nullptr)
        return false;
    else if (sig->getSize() == 0)
        return false;
    else
        return true;
}

void Equation::decreaseOperandCountInternal()
{
    clearOperand(allowedOperandCount-1);
    this->allowedOperandCount--;

    this->signalOperands.resize(this->allowedOperandCount);
    this->equationOperands.resize(this->allowedOperandCount);
}

void Equation::increaseOperandCountInternal()
{
    this->allowedOperandCount++;

    this->signalOperands.resize(this->allowedOperandCount);
    this->equationOperands.resize(this->allowedOperandCount);
}

uint Equation::getOperandCount() const
{
    if (this->function != nature::constant)
        return allowedOperandCount;
    else return 0;
}

bool Equation::increaseOperandCount()
{
    bool result = false;

    switch (function)
    {
    case nature::andOp:
    case nature::orOp:
    case nature::xorOp:
    case nature::nandOp:
    case nature::norOp:
    case nature::xnorOp:
    case nature::concatOp:
        this->increaseOperandCountInternal();

        emit signalStaticConfigurationChangedEvent();

        this->computeCurrentValue();

        result = true;
        break;
    case nature::notOp:
    case nature::identity:
    case nature::equalOp:
    case nature::diffOp:
    case nature::extractOp:
    case nature::constant:
        break;
    }

    return result;
}

bool Equation::decreaseOperandCount()
{
    bool result = false;

    switch (function)
    {
    case nature::andOp:
    case nature::orOp:
    case nature::xorOp:
    case nature::nandOp:
    case nature::norOp:
    case nature::xnorOp:
    case nature::concatOp:
        if (  (this->allowedOperandCount > 2) )
        {
            this->decreaseOperandCountInternal();

            emit signalStaticConfigurationChangedEvent();

            this->computeCurrentValue();

            result = true;
        }

        break;
    case nature::notOp:
    case nature::identity:
    case nature::equalOp:
    case nature::diffOp:
    case nature::extractOp:
    case nature::constant:
        break;
    }

    return result;
}
