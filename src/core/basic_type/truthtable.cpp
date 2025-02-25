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

// Current class header
#include "truthtable.h"

// C++ classes
#include <math.h>

// StateS classes
#include "statestypes.h"
#include "variable.h"
#include "equation.h"
#include "logicvalue.h"
#include "statesexception.h"
#include "exceptiontypes.h"
#include "operand.h"


TruthTable::TruthTable(shared_ptr<Equation> equation)
{
	QVector<shared_ptr<Equation>> equations;
	equations.append(equation);

	buildTable(equations);
}

TruthTable::TruthTable(QList<shared_ptr<Equation> > equations)
{
	buildTable(equations.toVector());
}

/**
 * @brief TruthTable::getInputs
 * @return Obtain the list of variables representing the
 * inputs of the truth table.
 */
QVector<shared_ptr<Variable> > TruthTable::getInputs() const // Throws StatesException
{
	QVector<shared_ptr<Variable>> list;

	for (auto& variable : this->inputVariablesTable)
	{
		if (! variable.expired())
			list.append(variable.lock());
		else
			throw StatesException("TruthTable", TruthTableError_t::reference_expired, "Reference to expired variable");
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
 * being a vector with each value corresponding to a input variable.
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

	for (QVector<LogicValue> currentRow : this->outputValuesTable)
	{
		output.append(currentRow[0]);
	}

	return output;
}

uint TruthTable::getInputCount() const
{
	return this->inputVariablesTable.count();
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
 * @brief TruthTable::extractVariables
 * @param equation
 * @return A list of all variables involved in equation, except
 * constants. Note that a variable can have multiple instances
 * in output list if it is present at multiple times in the equation.
 */
QList<shared_ptr<Variable>> TruthTable::extractVariables(shared_ptr<Equation> equation) const
{
	QList<shared_ptr<Variable>> list;

	int operandCount = equation->getOperandCount();
	for (int i = 0 ; i < operandCount ; i++)
	{
		auto operand = equation->getOperand(i);
		if (operand == nullptr) continue;


		if (operand->getSource() == OperandSource_t::equation)
		{
			list += extractVariables(operand->getEquation());
		}
		else if (operand->getSource() == OperandSource_t::variable)
		{
			list.append(operand->getVariable());
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
	// Obtain all variables involved in all equations
	QList<shared_ptr<Variable>> variablesList;

	for (auto& equation : equations)
	{
		variablesList += extractVariables(equation);
		this->outputEquationsTextsTable.append(equation->getText());
	}

	// Clean list so each variable only appears once
	QVector<shared_ptr<Variable>> variablesVector;

	for (auto& variable : variablesList)
	{
		if (!variablesVector.contains(variable))
		{
			variablesVector.append(variable);
			this->inputVariablesTable.append(variable);
		}
	}

	// Count inputs bit by bit
	uint inputCount = 0;
	for (auto& variable : variablesVector)
	{
		inputCount += variable->getSize();
	}

	// Prepare first row
	QVector<LogicValue> currentRow;
	for (auto& variable : variablesVector)
	{
		currentRow.append(LogicValue(variable->getSize(), false));
	}

	for (uint i = 0 ; i < pow(2, inputCount) ; i++)
	{
		// Add current row
		this->inputValuesTable.append(currentRow);

		// Compute outputs for this row
		for (int i = 0 ; i < variablesVector.count() ; i++)
		{
			variablesVector[i]->setCurrentValue(currentRow[i]); // Throws StatesException - value is built for variable size - ignored
		}

		QVector<LogicValue> currentResultLine;
		for (auto& equation : equations)
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
