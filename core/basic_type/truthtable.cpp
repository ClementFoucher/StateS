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
#include "truthtable.h"

// C++ classes
#include <math.h>

// StateS classes
#include "signal.h"
#include "equation.h"
#include "logicvalue.h"
#include "constant.h"
#include "statesexception.h"


TruthTable::TruthTable(shared_ptr<Equation> equation)
{
    QList<shared_ptr<Equation>> equations;
    equations.append(equation);

    buildTable(equations.toVector());
}

TruthTable::TruthTable(QList<shared_ptr<Equation> > equations)
{
    buildTable(equations.toVector());
}

/**
 * @brief TruthTable::getInputs
 * @return Obtain the list of signals representing the
 * inputs of the truth table.
 */
QVector<shared_ptr<Signal> > TruthTable::getInputs() const // Throws StatesException
{
    QVector<shared_ptr<Signal>> list;

    foreach(weak_ptr<Signal> sig, this->inputSignalsTable)
    {
        if (! sig.expired())
            list.append(sig.lock());
        else
            throw StatesException("TruthTable", reference_expired, "Reference to expired signal");
    }

    return list;
}

/**
 * @brief TruthTable::getOutputsEquations
 * @return The list of equations composing the outputs.
 */
QVector<QString> TruthTable::getOutputsEquations() const
{
    return this->outputEquationsTextsTable;
}

/**
 * @brief TruthTable::getInputTable
 * @return The list of inputs: a vector of rows, each row
 * being a vector with each value corresponding to a input signal.
 */
QVector<QVector<LogicValue> > TruthTable::getInputTable() const
{
    return this->inputValuesTable;
}

/**
 * @brief TruthTable::getOutputTable
 * @return A list of all outputs for each line of the
 * input table. It's a vector of rows, each line being
 * a vector itself containing a column for each output.
 */
QVector<QVector<LogicValue> > TruthTable::getOutputTable() const
{
    return this->outputValuesTable;
}

/**
 * @brief TruthTable::getSingleOutputTable
 * @return If there is only one output, get a simplified (vertical) vector.
 */
QVector<LogicValue> TruthTable::getSingleOutputTable() const
{
    QVector<LogicValue> output;

    foreach(QVector<LogicValue> currentRow, this->outputValuesTable)
    {
        output.append(currentRow[0]);
    }

    return output;
}

uint TruthTable::getInputCount() const
{
    return this->inputSignalsTable.count();
}

/**
 * @brief TruthTable::getOutputCount
 * @return The number of outputs.
 */
uint TruthTable::getOutputCount() const
{
    return this->outputEquationsTextsTable.count();
}

/**
 * @brief TruthTable::extractSignals
 * @param equation
 * @return A list of all signals involved in equation, except
 * constants. Note that a signal can have multiple instances
 * in output list if it is present at multiple times in the equation.
 */
QList<shared_ptr<Signal>> TruthTable::extractSignals(shared_ptr<Equation> equation) const
{
    QList<shared_ptr<Signal>> list;

    foreach(shared_ptr<Signal> sig, equation->getOperands())
    {
        shared_ptr<Equation> complexOperand = dynamic_pointer_cast<Equation>(sig);

        if (complexOperand != nullptr)
        {
            if (complexOperand->getFunction() != Equation::nature::constant)
                list += extractSignals(complexOperand);
        }
        else
        {
            if (dynamic_pointer_cast<Constant>(sig) == nullptr)
                list.append(sig);
        }
    }

    return list;
}

/**
 * @brief TruthTable::buildTable builds the table.
 * @param equations
 */
void TruthTable::buildTable(QVector<shared_ptr<Equation> > equations)
{
    // Obtain all signals involved in all equations
    QList<shared_ptr<Signal>> signalList;

    foreach(shared_ptr<Equation> equation, equations)
    {
        signalList += extractSignals(equation);
        this->outputEquationsTextsTable.append(equation->getText());
    }

    // Clean list so each signal only appears once
    QVector<shared_ptr<Signal>> signalVector;

    foreach(shared_ptr<Signal> signal, signalList)
    {
        if (!signalVector.contains(signal))
        {
            signalVector.append(signal);
            this->inputSignalsTable.append(signal);
        }
    }

    // Count inputs bit by bit
    uint inputCount = 0;
    foreach(shared_ptr<Signal> sig, signalVector)
    {
        inputCount += sig->getSize();
    }

    // Prepare first row
    QVector<LogicValue> currentRow;
    foreach(shared_ptr<Signal> sig, signalVector)
    {
        currentRow.append(LogicValue(sig->getSize(), false));
    }

    for (uint i = 0 ; i < pow(2, inputCount) ; i++)
    {
        // Add current row
        this->inputValuesTable.append(currentRow);

        // Compute outputs for this row
        for (int i = 0 ; i < signalVector.count() ; i++)
        {
            signalVector[i]->setCurrentValue(currentRow[i]); // Throws StatesException - value is built for signal size - ignored
        }

        QVector<LogicValue> currentResultLine;
        foreach(shared_ptr<Equation> equation, equations)
        {
            currentResultLine.append(equation->getCurrentValue());
        }
        this->outputValuesTable.append(currentResultLine);

        // Prepare next row
        for (int i = currentRow.count() - 1 ; i >= 0 ; i--)
        {
            bool carry = currentRow[i].increment();
            if (!carry)
                break;
        }
    }
}
