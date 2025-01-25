/*
 * Copyright © 2014-2025 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QVector>

// StateS classes
class Variable;
class LogicValue;
class Equation;


class TruthTable
{

	/////
	// Constructors/destructors
public:
	explicit TruthTable(shared_ptr<Equation> equation);
	explicit TruthTable(QList<shared_ptr<Equation>> equations);

	/////
	// Object functions
public:
	QVector<shared_ptr<Variable>> getInputs()            const; // Throws StatesException
	QVector<QVector<LogicValue>>  getInputTable()        const;
	QVector<QString>              getOutputsEquations()  const;
	QVector<QVector<LogicValue>>  getOutputTable()       const;
	QVector<LogicValue>           getSingleOutputTable() const;

	uint getInputCount()  const;
	uint getOutputCount() const;

private:
	QList<shared_ptr<Variable> > extractVariables(shared_ptr<Equation> equation) const;
	void buildTable(QVector<shared_ptr<Equation>> equations);

	/////
	// Object variables
private:
	QVector<weak_ptr<Variable>>  inputVariablesTable;
	QVector<QString>             outputEquationsTextsTable;
	QVector<QVector<LogicValue>> inputValuesTable;
	QVector<QVector<LogicValue>> outputValuesTable;

};

#endif // TRUTHTABLE_H
