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

#ifndef TRUTHTABLE_H
#define TRUTHTABLE_H

// Qt classes
#include <QVector>
#include <QList>
#include <QString>

// StateS classes
class Signal;
class LogicValue;
class Equation;


class TruthTable
{
public:
    explicit TruthTable(Equation* equation);
    explicit TruthTable(QList<Equation*> equations);

    QVector<Signal*>             getSignals() const;
    QVector<QVector<LogicValue>> getInputTable() const;
    QVector<QString>             getEquationTable() const;
    QVector<QVector<LogicValue>> getOutputTable() const;
    QVector<LogicValue>          getSingleOutputTable() const;

    uint getOutputCount() const;

private:
    QSet<Signal*> extractSignals(Equation* equation) const;
    void buildTable(QVector<Equation*> equations);

    QVector<Signal*>             signalTable;
    QVector<QVector<LogicValue>> inputTable;
    QVector<QString>             equationTable;
    QVector<QVector<LogicValue>> outputTable;
};

#endif // TRUTHTABLE_H
