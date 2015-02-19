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
    buildTable(equation);
}

QVector<Signal*> TruthTable::getSignals() const
{
    return this->signalTable;
}

QVector<QVector<LogicValue> > TruthTable::getInputTable() const
{
    return this->inputTable;
}

QVector<LogicValue> TruthTable::getOutputTable() const
{
    return this->outputTable;
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

void TruthTable::buildTable(Equation* equation)
{
    signalTable = (extractSignals(equation).toList()).toVector();

    uint inputCount = 0;
    foreach(Signal* sig, signalTable)
    {
        inputCount += sig->getSize();
    }

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
        outputTable.append(equation->getCurrentValue());

        // Prepare next row
        for (int i = currentRow.count() - 1 ; i >= 0 ; i--)
        {
            bool carry = currentRow[i].increment();
            if (!carry)
                break;
        }
    }
}












