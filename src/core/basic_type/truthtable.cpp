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

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "equation.h"
#include "operand.h"
#include "logicvalue.h"


TruthTable::TruthTable(shared_ptr<const Equation> equation)
{
	QList<shared_ptr<const Equation>> equations;
	equations.append(equation);

	this->buildTable(equations);
}

TruthTable::TruthTable(QList<shared_ptr<const Equation>> equations)
{
	this->buildTable(equations);
}

QString TruthTable::getInputVariableText(uint column) const
{
	if (column >= this->inputVariablesTexts.count()) return QString();


	return this->inputVariablesTexts.at(column);
}

QString TruthTable::getOutputEquationText(uint column) const
{
	if (column >= this->outputEquationsTexts.count()) return QString();


	return this->outputEquationsTexts.at(column);
}

LogicValue TruthTable::getInputValue(uint row, uint column) const
{
	if (row >= this->inputValuesTable.count()) return LogicValue::getNullValue();

	auto selectedRow = this->inputValuesTable.at(row);
	if (column >= selectedRow.count()) return LogicValue::getNullValue();


	return selectedRow.at(column);
}

LogicValue TruthTable::getOutputValue(uint row, uint column) const
{

	if (row >= this->outputValuesTable.count()) return LogicValue::getNullValue();

	auto selectedRow = this->outputValuesTable.at(row);
	if (column >= selectedRow.count()) return LogicValue::getNullValue();


	return selectedRow.at(column);
}

uint TruthTable::getRowsCount() const
{
	return this->inputValuesTable.count();
}

uint TruthTable::getInputCount() const
{
	return this->inputVariablesTexts.count();
}

uint TruthTable::getOutputCount() const
{
	return this->outputEquationsTexts.count();
}

/**
 * @brief TruthTable::extractVariables
 * @param equation
 * @return A list of all variables involved in equation, except
 * constants. Note that a variable can have multiple instances
 * in output list if it is present at multiple times in the equation.
 */
const QList<componentId_t> TruthTable::extractVariables(shared_ptr<const Equation> equation) const
{
	QList<componentId_t> list;
	if (equation == nullptr) return list;


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

void TruthTable::replaceVariableByConstant(shared_ptr<Equation> equation, componentId_t variableId, LogicValue constantValue) const
{
	if (equation == nullptr) return;


	for (uint i = 0 ; i < equation->getOperandCount() ; i++)
	{
		auto operand = equation->getOperand(i);
		if (operand == nullptr) continue;


		auto operandSource = operand->getSource();
		if (operandSource == OperandSource_t::variable)
		{
			if (operand->getVariableId() == variableId)
			{
				equation->setOperand(i, constantValue);
			}
		}
		else if (operandSource == OperandSource_t::equation)
		{
			this->replaceVariableByConstant(operand->getEquation(), variableId, constantValue);
		}
	}
}

void TruthTable::buildTable(QList<shared_ptr<const Equation>> equations)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	if (equations.count() == 0) return;


	// Obtain IDs of all variables involved in all equations
	// and build output texts table
	QList<componentId_t> variablesIdsList;
	for (auto& equation : equations)
	{
		if (equation == nullptr) continue;


		variablesIdsList += extractVariables(equation);
		this->outputEquationsTexts.append(equation->getText());
	}

	// Get a list of variables involved in the equation
	// and build input texts table
	QList<shared_ptr<Variable>> variablesList;
	for (auto& variableId : variablesIdsList)
	{
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) continue;


		if (variablesList.contains(variable) == false) // Make sure a variable is listed only once
		{
			variablesList.append(variable);
			this->inputVariablesTexts.append(variable->getName());
		}
	}

	// Compute total bits count from all inputs
	uint inputBitsCount = 0;
	for (auto& variable : variablesList)
	{
		inputBitsCount += variable->getSize();
	}

	// Prepare input values table first row
	QList<LogicValue> currentInputRow;
	for (auto& variable : variablesList)
	{
		currentInputRow.append(LogicValue(variable->getSize(), false));
	}

	// Build input and output values tables
	for (uint currentRowRank = 0 ; currentRowRank < pow(2, inputBitsCount) ; currentRowRank++)
	{
		// Register current input values row
		this->inputValuesTable.append(currentInputRow);

		// Compute outputs values for this row
		QList<LogicValue> currentOutputRow;
		for (auto& equation : equations)
		{
			// Build an equation in which we replace variables with constants
			// whose value is given by the current input values row
			auto clonedEquation = equation->clone();
			for (int inputRank = 0 ; inputRank < variablesList.count() ; inputRank++)
			{
				auto variableId = variablesList.at(inputRank)->getId();
				auto variableCurrentValue = currentInputRow[inputRank];

				this->replaceVariableByConstant(clonedEquation, variableId, variableCurrentValue);
			}
			currentOutputRow.append(clonedEquation->getInitialValue());
		}
		this->outputValuesTable.append(currentOutputRow);

		// Prepare input values table next row
		for (int inputRank = currentInputRow.count() - 1 ; inputRank >= 0 ; inputRank--)
		{
			bool carry = currentInputRow[inputRank].increment();
			if (carry == false)
			{
				break;
			}
		}
	}
}
