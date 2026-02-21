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

// Current class header
#include "truthtable.h"

// Qt classes
#include <QSet>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "equation.h"
#include "operand.h"
#include "logicvalue.h"


TruthTable::TruthTable(shared_ptr<const Equation> equation)
{
	if (equation != nullptr)
	{
		this->equations.append(equation);
	}

	this->prepareTable();
}

TruthTable::TruthTable(QList<shared_ptr<const Equation>> equations)
{
	for (auto& equation : equations)
	{
		if (equation != nullptr)
		{
			this->equations.append(equation);
		}
	}

	this->prepareTable();
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
	if (this->tableBuilt == false) return LogicValue::getNullValue();

	if (row >= this->inputValuesTable.count()) return LogicValue::getNullValue();

	auto selectedRow = this->inputValuesTable.at(row);
	if (column >= selectedRow.count()) return LogicValue::getNullValue();


	return selectedRow.at(column);
}

LogicValue TruthTable::getOutputValue(uint row, uint column) const
{
	if (this->tableBuilt == false) return LogicValue::getNullValue();

	if (row >= this->outputValuesTable.count()) return LogicValue::getNullValue();

	auto selectedRow = this->outputValuesTable.at(row);
	if (column >= selectedRow.count()) return LogicValue::getNullValue();


	return selectedRow.at(column);
}

uint TruthTable::getRowsCount() const
{
	return this->rowsCount;
}

uint TruthTable::getInputCount() const
{
	return this->inputVariablesTexts.count();
}

uint TruthTable::getOutputCount() const
{
	return this->outputEquationsTexts.count();
}

bool TruthTable::getTableBuiltSuccessfully() const
{
	return this->tableBuilt;
}

void TruthTable::buildTable()
{
	if (this->rowsCount == 0) return;


	while (this->tableBuilt == false)
	{
		this->buildRow();
	}
}

/**
 * @brief TruthTable::buildRow builds a single row of the table.
 *        Each call to this function will add a row to the table.
 *        The function will return true when the last row has been built.
 * @return true if all rows have been built, false if there are
 *         rows remaining to be built.
 */
bool TruthTable::buildRow()
{
	if (this->rowsCount == 0) return true;

	if (this->tableBuilt == true) return true;


	if (this->currentRowRank == 0)
	{
		// Prepare input values table first row
		for (auto& variable : this->variablesList)
		{
			this->currentInputRow.append(LogicValue(variable->getSize(), false));
		}
	}

	// Register current input values row
	this->inputValuesTable.append(this->currentInputRow);

	// Compute outputs values for this row
	QList<LogicValue> currentOutputRow;
	for (auto& equation : this->equations)
	{
		// Build an equation in which we replace variables with constants
		// whose value is given by the current input values row
		auto clonedEquation = equation->clone();
		for (int inputRank = 0 ; inputRank < this->variablesList.count() ; inputRank++)
		{
			auto variableId = this->variablesList.at(inputRank)->getId();
			auto variableCurrentValue = this->currentInputRow[inputRank];

			this->replaceVariableByConstant(clonedEquation, variableId, variableCurrentValue);
		}
		currentOutputRow.append(clonedEquation->getInitialValue());
	}
	this->outputValuesTable.append(currentOutputRow);

	// Prepare input values table next row
	for (int inputRank = this->currentInputRow.count() - 1 ; inputRank >= 0 ; inputRank--)
	{
		bool carry = this->currentInputRow[inputRank].increment();
		if (carry == false)
		{
			break;
		}
	}

	this->currentRowRank++;
	if (this->currentRowRank == this->rowsCount)
	{
		this->tableBuilt = true;
		this->currentInputRow.clear();
		this->equations.clear();
		this->variablesList.clear();
	}

	return this->tableBuilt;
}

void TruthTable::prepareTable()
{
	// this->equations is guaranteed to contain no nullptr
	// when this function is called

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	if (this->equations.count() == 0) return;


	// Obtain IDs of all variables involved in all equations
	QSet<componentId_t> variablesIds;
	for (auto& equation : this->equations)
	{
		variablesIds += equation->getVariablesIdsSet();
	}

	if (variablesIds.count() == 0)
	{
		this->equations.clear();
		return;
	}

	// Get a pointer to variables involved in the equation
	for (auto& variableId : std::as_const(variablesIds))
	{
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) continue;


		this->variablesList.append(variable);
	}

	// Compute total bits count from all inputs
	uint inputBitsCount = 0;
	for (auto& variable : this->variablesList)
	{
		inputBitsCount += variable->getSize();
	}

	// Make sure equation is computable
	if (inputBitsCount <= 15)
	{
		this->rowsCount = pow(2, inputBitsCount);
	}
	else
	{
		this->equations.clear();
		this->variablesList.clear();
		return;
	}

	// Build input texts table
	for (auto& variable : this->variablesList)
	{
		this->inputVariablesTexts.append(variable->getName());
	}

	// Build output texts table
	for (auto& equation : this->equations)
	{
		this->outputEquationsTexts.append(equation->getText());
	}
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
