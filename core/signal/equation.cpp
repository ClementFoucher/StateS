/*
 * Copyright © 2014-2016 Clément Foucher
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
#include <QMetaMethod>

// StateS classes
#include "statesexception.h"


// Main constructor
/**
 * @brief Equation::Equation
 * @param function
 * @param operandCount Number of operands. Omitting this value results in a default-sized equation with 2 operands for variable operand functions.
 */
Equation::Equation(nature function, int operandCount) :
    Signal("<sub>(equation)</sub>")
{
    this->function = function;
    this->currentValue = LogicValue::getNullValue();

    // Operand count affectation
    switch(this->function)
    {
    case nature::constant:
        this->allowedOperandCount = 0;

        if ( (operandCount != 0) && (operandCount >= 0) )
        {
            qDebug() << "(Equation:) Warning: Trying to create constant equation with size != 0 (requested size is " << QString::number(operandCount) << ").";
            qDebug() << "Requested size value ignored and set to 0.";
        }
        break;
    case nature::extractOp:
    case nature::notOp:
    case nature::identity:
        this->allowedOperandCount = 1;

        if ( (operandCount != 1) && (operandCount >= 0) )
        {
            qDebug() << "(Equation:) Warning: Trying to create a fixed size 1 equation with size != 1 (requested size is " << QString::number(operandCount) << ").";
            qDebug() << "Requested size value ignored and set to 1.";
        }
        break;
    case nature::equalOp:
    case nature::diffOp:
        this->allowedOperandCount = 2;

        if ( (operandCount != 2) && (operandCount >= 0) )
        {
            qDebug() << "(Equation) Warning: Trying to create a fixed size 2 equation with size != 2 (requested size is " << QString::number(operandCount) << ").";
            qDebug() << "Requested size value ignored and set to 2.";
        }
        break;
    case nature::andOp:
    case nature::orOp:
    case nature::xorOp:
    case nature::nandOp:
    case nature::norOp:
    case nature::xnorOp:
    case nature::concatOp:
        if (operandCount >= 2)
        {
            this->allowedOperandCount = operandCount;
        }
        else if (operandCount < 0)
        {
            this->allowedOperandCount = 2;
        }
        else
        {
            this->allowedOperandCount = 2;

            qDebug() << "(Equation) Warning: Trying to create an equation with size < 2 (requested size is " << QString::number(operandCount) << ").";
            qDebug() << "Requested size value ignored and set to 2.";
        }
    }

    // Default values
    this->rangeL = -1;
    this->rangeR = -1;
    this->constantValue = LogicValue::getNullValue();

    this->signalOperands   = QVector<weak_ptr<Signal>>    (this->allowedOperandCount);
    this->equationOperands = QVector<shared_ptr<Equation>>(this->allowedOperandCount);
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
    shared_ptr<Equation> eq = shared_ptr<Equation>(new Equation(this->function, this->getOperands()));

    if (this->function == Equation::nature::constant)
    {
        eq->setConstantValue(this->constantValue); // Throws StatesException - constantValue is built for signal size - ignored
    }
    else if (this->function == Equation::nature::extractOp)
    {
        eq->setRange(this->rangeL, this->rangeR);
    }


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

void Equation::resize(uint) // Throws StatesException
{
    // Equation size is dynamic (or is 1 for eq and diff).
    // Can't be resized.
    throw StatesException("Equation", resized_requested, "Trying to resize an equation");
}

Equation::nature Equation::getFunction() const
{
    return function;
}

void Equation::setFunction(const nature& newFunction)
{
    switch(newFunction)
    {
    case nature::notOp:
    case nature::identity:
    case nature::extractOp:
        // Delete operands beyond one
        while (this->allowedOperandCount > 1)
        {
            // Muted: do not causes recomputation
            decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored
        }
        break;
    case nature::equalOp:
    case nature::diffOp:
        // Exactly two operands needed

        // Delete operands beyond two
        while (this->allowedOperandCount > 2)
        {
            decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored
        }

        // Add operand if not enough
        if (allowedOperandCount == 1)
        {
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
            increaseOperandCountInternal();
        }
        break;
    case nature::constant:
        // Delete operands
        while (allowedOperandCount > 0)
        {
            decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored
        }
        break;
    }

    this->function = newFunction;

    if (this->function == nature::extractOp)
    {
        this->rangeL = -1;
        this->rangeR = -1;
    }

    emit signalStaticConfigurationChangedEvent();

    this->computeCurrentValue();
}

void Equation::setRange(int rangeL, int rangeR)
{
    if (this->function == nature::extractOp)
    {
        this->rangeL = rangeL;
        this->rangeR = rangeR;

        emit signalStaticConfigurationChangedEvent();

        this->computeCurrentValue();
    }

}

shared_ptr<Signal> Equation::getOperand(uint i) const // Throws StatesException
{
    if (i < this->allowedOperandCount)
        return this->getOperands()[i];
    else
        throw StatesException("Equation", out_of_range_access, "Out of range operand access");
}

/**
 * @brief Equation::setOperand
 * @param i
 * @param newOperand
 * Equation always owns its own Equation operands.
 * All given operands that are equation will be cloned.
 * @return
 */
bool Equation::setOperand(uint i, shared_ptr<Signal> newOperand, bool quiet) // Throws StatesException
{
    // Do not allow placing an operand outside defined range
    if (i >= this->allowedOperandCount)
    {
        throw StatesException("Equation", out_of_range_access, "Out of range operand access");
    }

    // Only proceed to set if new operand is not the same as the current one.
    // If it is, just return success
    if (this->getOperand(i) == newOperand) // Throws StatesException - Operand count checked - ignored
    {
        return true;
    }
    else
    {
        // Muted: do not causes recomputation
        clearOperand(i, true); // Throws StatesException - Operand count checked - ignored

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
                connect(newOperand.get(), &Signal::signalDeletedEvent, this, &Equation::signalDeletedEventHandler);
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

void Equation::clearOperand(uint i, bool quiet) // Throws StatesException
{
    // Chek index
    if (i >= this->allowedOperandCount)
    {
        throw StatesException("Equation", out_of_range_access, "Out of range operand access");
    }

    shared_ptr<Signal> oldOperand = this->getOperand(i); // Throws StatesException - Operand count checked - ignored

    if (oldOperand != nullptr)
    {
        // Signal propagation
        disconnect(oldOperand.get(), &Signal::signalStaticConfigurationChangedEvent, this, &Signal::signalStaticConfigurationChangedEvent);
        // Local stuff
        disconnect(oldOperand.get(), &Signal::signalStaticConfigurationChangedEvent, this, &Equation::computeCurrentValue);
        disconnect(oldOperand.get(), &Signal::signalDynamicStateChangedEvent,        this, &Equation::computeCurrentValue);

        if (oldOperand != nullptr)
        {
            disconnect(oldOperand.get(), &Signal::signalDeletedEvent, this, &Equation::signalDeletedEventHandler);
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

QString Equation::getText() const
{
    return this->getColoredText(false, false);
}

QString Equation::getColoredText(bool activeColored, bool errorColored) const
{
    QString errorTextBegin;
    QString errorTextEnd;

    if ( (this->getSize() == 0) && (errorColored == true) )
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
            shared_ptr<Signal> signalOperand = getOperand(i); // Throws StatesException - Contrained by operand count - ignored
            shared_ptr<Equation> equationOperand = dynamic_pointer_cast<Equation>(signalOperand);

            if (equationOperand != nullptr)
            {
                text += equationOperand->getColoredText(activeColored, false);
            }
            else if (signalOperand != nullptr)
            {
                text += signalOperand->getColoredText(activeColored);
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

            if (this->rangeL != -1)
                text += QString::number(this->rangeL);
            else
                text += "…";

            if (this->rangeR != -1)
            {
                text += ".." + QString::number(this->rangeR);
            }

            text += "]";
        }

        if (this->allowedOperandCount > 1)
            text += " )";
    }

    text += errorTextEnd;

    return text;
}

void Equation::setConstantValue(const LogicValue& value) // Throws StatesException
{
    if (this->function == nature::constant)
    {
        this->constantValue = value;

        this->computeCurrentValue();

        emit Signal::signalStaticConfigurationChangedEvent();
    }
    else
        throw StatesException("Equation", set_value_requested, "Trying to affect a value to a dynamically determined equation");
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

int Equation::getRangeL() const
{
    return this->rangeL;
}

int Equation::getRangeR() const
{
    return this->rangeR;
}

void Equation::setInitialValue(const LogicValue&)
{
    throw StatesException("Equation", set_value_requested, "Trying to affect an initial value to an equation");
}

void Equation::setCurrentValue(const LogicValue&)
{
    throw StatesException("Equation", set_value_requested, "Trying to affect a current value to an equation");
}

void Equation::setCurrentValueSubRange(const LogicValue&, int, int)
{
    throw StatesException("Equation", set_value_requested, "Trying to affect a current value to an equation");
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
    // Current value is computed dynamically when operands or
    // equation change.
    // It emits change events only if value actually changed

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
            if (this->rangeL != -1)
            {
                if (this->rangeR != -1)
                {
                    if (this->rangeL >= this->rangeR)
                    {
                        int range = this->rangeL - this->rangeR + 1;
                        LogicValue subVector(range);
                        LogicValue originalValue = operands[0]->getCurrentValue();

                        for (int i = 0 ; i < range ; i++)
                        {
                            subVector[i] = originalValue[this->rangeR + i];
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
                    if ((uint)rangeL < operands[0]->getSize())
                    {
                        LogicValue result(1);
                        result[0] = operands[0]->getCurrentValue()[rangeL];
                        this->currentValue = result;
                    }
                    else
                    {
                        rangeL = -1;
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
            this->currentValue = this->constantValue;
            break;
        }
    }
    else
    {
        this->currentValue = LogicValue::getNullValue();
    }

    if (previousValue != this->currentValue)
        emit signalDynamicStateChangedEvent();

    if (previousValue.getSize() != this->currentValue.getSize())
    {
        try
        {
            emit signalResizedEvent();
        }
        catch (const std::bad_weak_ptr&)
        {
            // It's ok to fail here: it means shared_from_this is illegal to call.
            // This happens when this function is called from the constructor
        }
    }
}

void Equation::signalDeletedEventHandler()
{
    this->computeCurrentValue();
    emit this->signalStaticConfigurationChangedEvent();
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

/**
 * @brief Equation::decreaseOperandCountInternal Decreases the operand count without
 * checking on the function required operands. It still makes sure the operand count
 * doesn't go below 0.
 * This operation can place the object in a state where function does not have it
 * required count of operand, so the caller has to make sure he replaces the object
 * in a correct state before any call to value computing is made.
 */
void Equation::decreaseOperandCountInternal() // Throws StatesException
{
    // Do not allow placing an operand outside defined range
    if (this->allowedOperandCount == 0)
    {
        throw StatesException("Equation", reduced_operand_while_0, "Trying to reduce operand count while count is 0");
    }

    clearOperand(this->allowedOperandCount-1); // Throws StatesException - Operand count checked - ignored
    this->allowedOperandCount--;

    this->signalOperands.resize(this->allowedOperandCount);
    this->equationOperands.resize(this->allowedOperandCount);
}

/**
 * @brief Equation::increaseOperandCountInternal Increases the operand count without
 * checking on the function required operands.
 * This operation can place the object in a state where function does not have it
 * required count of operand, so the caller has to make sure he replaces the object
 * in a correct state before any call to value computing is made.
 */
void Equation::increaseOperandCountInternal()
{
    this->allowedOperandCount++;

    this->signalOperands.resize(this->allowedOperandCount);
    this->equationOperands.resize(this->allowedOperandCount);
}

uint Equation::getOperandCount() const
{
    return this->allowedOperandCount;
}

void Equation::increaseOperandCount() // Throws StatesException
{
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
        break;
    case nature::notOp:
    case nature::identity:
    case nature::equalOp:
    case nature::diffOp:
    case nature::extractOp:
    case nature::constant:
        throw StatesException("Equation", change_operand_illegal, "Trying to change the operand count on a fixed operand count function");
        break;
    }
}

void Equation::decreaseOperandCount() // Throws StatesException
{
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
            this->decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored

            emit signalStaticConfigurationChangedEvent();

            this->computeCurrentValue();
        }

        break;
    case nature::notOp:
    case nature::identity:
    case nature::equalOp:
    case nature::diffOp:
    case nature::extractOp:
    case nature::constant:
        throw StatesException("Equation", change_operand_illegal, "Trying to change the operand count on a fixed operand count function");
        break;
    }
}
