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
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "equation.h"
#include "logicvalue.h"
#include "operand.h"


TruthTable::TruthTable(shared_ptr<Equation> equation)
{
	QVector<shared_ptr<Equation>> equations;
	equations.append(equation);

	this->buildTable(equations);
}

TruthTable::TruthTable(QList<shared_ptr<Equation> > equations)
{
	this->buildTable(equations.toVector());
}

/**
 * @brief TruthTable::getInputVariablesIds
 * @return Obtain the list of variables representing the
 * inputs of the truth table.
 */
const QVector<componentId_t> TruthTable::getInputVariablesIds() const
{
	return this->inputVariablesIdsTable;
}

/**
 * @brief TruthTable::getOutputsEquations
 * @return The list of equations composing the outputs.
 */
const QVector<QString> TruthTable::getOutputsEquations() const
{
	return this->outputEquationsTextsTable;
}

/**
 * @brief TruthTable::getInputValuesTable
 * @return The list of inputs: a vector of rows, each row
 * being a vector with each value corresponding to an input variable.
 */
const QVector<QVector<LogicValue> > TruthTable::getInputValuesTable() const
{
	return this->inputValuesTable;
}

/**
 * @brief TruthTable::getOutputValuesTable
 * @return A list of all outputs for each line of the
 * input table. It's a vector of rows, each line being
 * a vector itself containing a column for each output.
 */
const QVector<QVector<LogicValue> > TruthTable::getOutputValuesTable() const
{
	return this->outputValuesTable;
}

/**
 * @brief TruthTable::getSingleOutputValuesTable
 * @return If there is only one output, get a simplified (vertical) vector.
 */
const QVector<LogicValue> TruthTable::getSingleOutputValuesTable() const
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
	return this->inputVariablesIdsTable.count();
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
const QList<componentId_t> TruthTable::extractVariables(shared_ptr<Equation> equation) const
{
	QList<componentId_t> list;

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
			list.append(operand->getVariableId());
		}
	}

	return list;
}

/**
 * @brief TruthTable::buildTable builds the table.
 * @param equations
 */
void TruthTable::buildTable(QVector<shared_ptr<Equation>> equations)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	// Obtain all variables involved in all equations
	QList<componentId_t> variablesIdsList;

	for (auto& equation : equations)
	{
		variablesIdsList += extractVariables(equation);
		this->outputEquationsTextsTable.append(equation->getText());
	}

	// Clean list so each variable only appears once
	QVector<shared_ptr<Variable>> variablesVector;

	for (auto& variableId : variablesIdsList)
	{
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) continue;


		if (!variablesVector.contains(variable))
		{
			variablesVector.append(variable);
			this->inputVariablesIdsTable.append(variableId);
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
			variablesVector[i]->setCurrentValue(currentRow[i]);
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
			if (carry == false)
			{
				break;
			}
		}
	}
}
