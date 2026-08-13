/*
 * Copyright © 2014-2026 Clément Foucher
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

// Stdlib
#include <memory>
using namespace std;

// Qt
#include <QList>

// StateS
#include "statestypes.h"
class MachineValue;
class Equation;
class Variable;


/**
 * @brief The TruthTable class represents a table containing all
 *        possible combinations of input values for a given
 *        equation and it corresponding outputs.
 *        When the object is built, the number of lines and columns
 *        in the table are computed, as well as their label texts.
 *        From there, the table can be built.
 *        Building the table can be done at once using the buildTable
 *        function, or row by row by calling the buildRow function
 *        multiple times until it returns true.
 *        Row-by-row building is provided so that the process can
 *        be interrupted before completion, as large tables can
 *        require quite an amount of time (and memory) to be built.
 *        The table will be considered invalid if the equation has
 *        more than 32768 rown (and this is probably far more than
 *        what a human being can read!).
 */
class TruthTable
{

	/////
	// Constructors/destructors
public:
	explicit TruthTable(shared_ptr<const Equation> equation);
	explicit TruthTable(QList<shared_ptr<const Equation>> equations);

	/////
	// Object functions
public:
	QString getInputVariableText (uint column) const;
	QString getOutputEquationText(uint column) const;

	MachineValue getInputValue (uint row, uint column) const;
	MachineValue getOutputValue(uint row, uint column) const;

	uint getRowsCount()   const;
	uint getInputCount()  const;
	uint getOutputCount() const;

	bool getTableBuiltSuccessfully() const;

	void buildTable();
	bool buildRow();

private:
	void prepareTable();

	void replaceVariableByConstant(shared_ptr<Equation> equation, componentId_t variableId, MachineValue constantValue) const;

	/////
	// Object variables
private:
	// Table status
	bool tableBuilt = false;

	// Members valid as soon as object has been created
	uint16_t rowsCount = 0;

	QList<QString> inputVariablesTexts;
	QList<QString> outputEquationsTexts;

	// Members valid only once the table has been built
	QList<QList<MachineValue>> inputValuesTable;
	QList<QList<MachineValue>> outputValuesTable;

	// Temporary members, useless and cleared once table has been built
	QList<shared_ptr<const Equation>> equations;
	QList<shared_ptr<const Variable>> variablesList;
	QList<MachineValue> currentInputRow;
	uint currentRowRank = 0;

};

#endif // TRUTHTABLE_H
