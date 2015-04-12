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
#include "truthtable.h"

// C++ classes
#include <math.h>

// StateS classes
#include "signal.h"
#include "equation.h"
#include "logicvalue.h"


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

QVector<shared_ptr<Signal> > TruthTable::getSignals() const
{
    QVector<shared_ptr<Signal>> list;

    foreach(weak_ptr<Signal> sig, this->signalTable)
    {
        shared_ptr<Signal> sigL = sig.lock();

        list.append(sigL);
    }

    return list;
}

QVector<QVector<LogicValue> > TruthTable::getInputTable() const
{
    return this->inputTable;
}

QVector<QString> TruthTable::getEquationTable() const
{
    return this->equationTable;
}

QVector<QVector<LogicValue> > TruthTable::getOutputTable() const
{
    return this->outputTable;
}

// If there is only one output, get a simplified (vertical) vector
QVector<LogicValue> TruthTable::getSingleOutputTable() const
{
    QVector<LogicValue> output;

    foreach(QVector<LogicValue> currentRow, this->outputTable)
    {
        output.append(currentRow[0]);
    }

    return output;
}

uint TruthTable::getOutputCount() const
{
    return this->outputTable.count();
}

QList<shared_ptr<Signal>> TruthTable::extractSignals(shared_ptr<Equation> equation) const
{
    QList<shared_ptr<Signal>> list;

    foreach(shared_ptr<Signal> sig, equation->getOperands())
    {
        shared_ptr<Equation> complexOperand = dynamic_pointer_cast<Equation>(sig);

        if (complexOperand != nullptr)
        {
            list += extractSignals(complexOperand);
        }
        else
        {
            if (!sig->getIsConstant())
                list.append(sig);
        }
    }

    return list;
}

void TruthTable::buildTable(QVector<shared_ptr<Equation> > equations)
{
    QList<shared_ptr<Signal>> signalList;

    foreach(shared_ptr<Equation> equation, equations)
    {
        signalList += extractSignals(equation);
        this->equationTable.append(equation->getText());
    }

    QVector<shared_ptr<Signal>> signalVector;

    foreach(shared_ptr<Signal> signal, signalList)
    {
        if (!signalVector.contains(signal))
        {
            signalVector.append(signal);
            this->signalTable.append(signal);
        }
    }

    //signalTable = (signalSet.toList()).toVector();

    uint inputCount = 0;
    foreach(shared_ptr<Signal> sig, signalVector)
    {
        inputCount += sig->getSize();
    }

    // Compute initial row
    QVector<LogicValue> currentRow;
    foreach(shared_ptr<Signal> sig, signalVector)
    {
        currentRow.append(LogicValue(sig->getSize(), false));
    }

    for (uint i = 0 ; i < pow(2, inputCount) ; i++)
    {
        // Get current row
        this->inputTable.append(currentRow);

        // Compute outputs for this row
        for (int i = 0 ; i < signalVector.count() ; i++)
        {
            signalVector[i]->setCurrentValue(currentRow[i]);
        }

        QVector<LogicValue> currentResultLine;
        foreach(shared_ptr<Equation> equation, equations)
        {
            currentResultLine.append(equation->getCurrentValue());
        }
        this->outputTable.append(currentResultLine);

        // Prepare next row
        for (int i = currentRow.count() - 1 ; i >= 0 ; i--)
        {
            bool carry = currentRow[i].increment();
            if (!carry)
                break;
        }
    }
}












