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

// Qt classes
#include <QSet>

// StateS classes
#include "signal.h"
#include "equation.h"
#include "logicvalue.h"


TruthTable::TruthTable(Equation* equation)
{
    QList<Equation*> equations;
    equations.append(equation);

    buildTable(equations.toVector());
}

TruthTable::TruthTable(QList<Equation*> equations)
{
    buildTable(equations.toVector());
}

QVector<Signal*> TruthTable::getSignals() const
{
    return this->signalTable;
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

QSet<Signal*> TruthTable::extractSignals(Equation* equation) const
{
    QSet<Signal*> list;

    foreach(Signal* sig, equation->getOperands())
    {
        Equation* complexOperand = dynamic_cast<Equation*>(sig);

        if (complexOperand != nullptr)
        {
            list += extractSignals(complexOperand);
        }
        else
        {
            list.insert(sig);
        }
    }

    return list;
}

void TruthTable::buildTable(QVector<Equation*> equations)
{
    QSet<Signal*> signalSet;

    foreach(Equation* equation, equations)
    {
        signalSet += extractSignals(equation);
        this->equationTable.append(equation->getText());
    }

    signalTable = (signalSet.toList()).toVector();

    uint inputCount = 0;
    foreach(Signal* sig, signalTable)
    {
        inputCount += sig->getSize();
    }

    // Compute initial row
    QVector<LogicValue> currentRow;
    foreach(Signal* sig, signalTable)
    {
        currentRow.append(LogicValue(sig->getSize(), false));
    }

    for (uint i = 0 ; i < pow(2, inputCount) ; i++)
    {
        // Get current row
        inputTable.append(currentRow);

        // Compute outputs for this row
        for (int i = 0 ; i < signalTable.count() ; i++)
        {
            signalTable[i]->setCurrentValue(currentRow[i]);
        }

        QVector<LogicValue> currentResultLine;
        foreach(Equation* equation, equations)
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












