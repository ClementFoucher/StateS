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
#include "equation.h"

// StateS classes
#include "statestypes.h"
#include "operand.h"
#include "variable.h"


/**
 * @brief Equation::Equation
 * @param operatorType Operator of the equation.
 * @param operandCount Number of operands. This currentValue is ignored
 * for operators that have a fixed-size operand count.
 * Omitting this currentValue for a variable-size operator results in
 * an equation with 2 operands for variable operand functions.
 */
Equation::Equation(OperatorType_t operatorType, int operandCount)
{
	this->operatorType = operatorType;
	this->failureCause = EquationComputationFailureCause_t::nullOperand;
	this->currentValue = LogicValue::getNullValue();

	// Compute operand count
	uint actualOperandCount;
	switch(this->operatorType)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::extractOp:
		actualOperandCount = 1;
		break;
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		actualOperandCount = 2;
		break;
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		if (operandCount >= 2)
		{
			actualOperandCount = operandCount;
		}
		else
		{
			actualOperandCount = 2;
		}
		break;
	}

	// Initialize with null operands
	for (uint i = 0 ; i < actualOperandCount ; i++)
	{
		this->operands.append(nullptr);
	}
}

shared_ptr<Equation> Equation::clone() const
{
	auto clonedEquation = shared_ptr<Equation>(new Equation(this->operatorType, this->operands.count()));

	for (uint i = 0 ; i < this->operands.count() ; i++)
	{
		auto operand = this->operands.at(i);
		if (operand != nullptr)
		{
			clonedEquation->setOperand(i, operand->clone());
		}
	}

	if (this->operatorType == OperatorType_t::extractOp)
	{
		clonedEquation->setRange(this->rangeL, this->rangeR);
	}

	return clonedEquation;
}

/**
 * @brief getSize returns the apparent size of the equation.
 * 0 means current size is not defined => result will be a null variable.
 * @return
 */
uint Equation::getSize() const
{
	return this->initialValue.getSize();
}

LogicValue Equation::getInitialValue() const
{
	return this->initialValue;
}

LogicValue Equation::getCurrentValue() const
{
	return this->currentValue;
}

QString Equation::getText() const
{
	return this->getColoredText(true);
}

QString Equation::getColoredText(bool raw) const
{
	QString text;
	if (raw == false)
	{
		if (this->getSize() == 0)
		{
			text += "<span style=\"color:red;\">";
		}
		else
		{
			text += "<span style=\"color:black;\">";
		}
	}

	uint operandCount = this->getOperandCount();

	// Inversion oeprator
	if (this->isInverted())
		text += '/';

	if (operandCount > 1)
		text += "( ";

	for (uint i = 0 ; i < operandCount ; i++)
	{
		auto operand = this->getOperand(i);
		if (operand == nullptr)
		{
			text += "…";
		}
		else
		{
			text += operand->getText();
		}

		// Add operator, except for last operand
		if (i < operandCount - 1)
		{
			switch(operatorType)
			{
			case OperatorType_t::andOp:
			case OperatorType_t::nandOp:
				text += " • ";
				break;
			case OperatorType_t::orOp:
			case OperatorType_t::norOp:
				text += " + ";
				break;
			case OperatorType_t::xorOp:
			case OperatorType_t::xnorOp:
				text += " ⊕ ";
				break;
			case OperatorType_t::equalOp:
				text += " = ";
				break;
			case OperatorType_t::diffOp:
				text += " ≠ ";
				break;
			case OperatorType_t::concatOp:
				text += " : ";
				break;
			case OperatorType_t::notOp:
			case OperatorType_t::identity:
			case OperatorType_t::extractOp:
				break;
			}
		}
	}

	if (this->operatorType == OperatorType_t::extractOp)
	{
		text += "[";

		if (this->rangeL != -1)
			text += QString::number(this->rangeL);
		else
			text += "…";

		if (this->rangeR != -1)
		{
			text += ".." + QString::number(this->rangeR);
		}

		text += "]";
	}

	if (operandCount > 1)
		text += " )";

	if (raw == false)
	{
		text += "</span>";
	}

	return text;
}

EquationComputationFailureCause_t Equation::getComputationFailureCause() const
{
	return this->failureCause;
}

OperatorType_t Equation::getOperatorType() const
{
	return this->operatorType;
}

bool Equation::isInverted() const
{
	switch (this->operatorType)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
		return true;
		break;
	case OperatorType_t::identity:
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::concatOp:
		return false;
		break;
	}
}

// True concept here only apply to one bit results
bool Equation::isTrue() const
{
	if (this->getSize() == 1)
	{
		if (this->currentValue == LogicValue::getValue1(1))
		{
			return true;
		}
	}

	return false;
}

shared_ptr<Operand> Equation::getOperand(uint i) const
{
	if (i < this->getOperandCount())
	{
		return this->operands.at(i);
	}
	else
	{
		return nullptr;
	}
}

void Equation::setOperand(uint i, componentId_t newOperand)
{
	auto operand = shared_ptr<Operand>(new Operand(newOperand));
	this->setOperand(i, operand);
}

void Equation::setOperand(uint i, shared_ptr<Equation> newOperand)
{
	auto operand = shared_ptr<Operand>(new Operand(newOperand));
	this->setOperand(i, operand);
}

void Equation::setOperand(uint i, LogicValue newOperand)
{
	auto operand = shared_ptr<Operand>(new Operand(newOperand));
	this->setOperand(i, operand);
}

void Equation::setOperand(uint i, shared_ptr<Variable> newOperand)
{
	auto operand = shared_ptr<Operand>(new Operand(newOperand));
	this->setOperand(i, operand);
}

void Equation::clearOperand(uint i)
{
	this->setOperand(i, shared_ptr<Operand>(nullptr));
}

uint Equation::getOperandCount() const
{
	switch(this->operatorType)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::extractOp:
		return 1;
		break;
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		return 2;
		break;
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		return this->operands.size();
		break;
	}
}

void Equation::increaseOperandCount()
{
	bool doIncrease = false;

	switch (this->operatorType)
	{
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		doIncrease = true;
		break;
	case OperatorType_t::extractOp:
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		// Do not allow modifying operand count for these fixed-size operators
		break;
	}

	if (doIncrease == true)
	{
		this->operands.append(nullptr);
		this->computeInitialValue();
		emit this->equationTextChangedEvent();
	}
}

void Equation::decreaseOperandCount()
{
	bool doDecrease = false;

	switch (this->operatorType)
	{
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		if (this->getOperandCount() > 2)
		{
			doDecrease = true;
		}
		break;
	case OperatorType_t::extractOp:
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		// Do not allow modifying operand count for these fixed-size operators
		break;
	}

	if (doDecrease == true)
	{
		this->operands.removeLast();
		this->computeInitialValue();
		emit this->equationTextChangedEvent();
	}
}

void Equation::setRange(int rangeL, int rangeR)
{
	if (this->operatorType == OperatorType_t::extractOp)
	{
		this->rangeL = rangeL;
		this->rangeR = rangeR;

		this->computeInitialValue();
		emit this->equationTextChangedEvent();
	}
}

int Equation::getRangeL() const
{
	return this->rangeL;
}

int Equation::getRangeR() const
{
	return this->rangeR;
}

void Equation::doFullStackRecomputation()
{
	for (auto& operand : this->operands)
	{
		if (operand == nullptr) continue;


		if (operand->getSource() == OperandSource_t::equation)
		{
			auto equation = operand->getEquation();
			if (equation != nullptr)
			{
				equation->doFullStackRecomputation();
			}
		}
	}

	this->computeInitialValue();
}

void Equation::computeCurrentValue()
{
	if (this->initialValue.isNull() == true)
	{
		this->currentValue = LogicValue::getNullValue();
		return;
	}


	LogicValue previousValue = this->currentValue;
	this->currentValue = this->computeValue(CurrentOrInitial_t::current);

	if (previousValue != this->currentValue)
	{
		emit this->equationCurrentValueChangedEvent();
	}
}

void Equation::computeInitialValue()
{
	bool doCompute = true;

	// Check for null or incomplete operands
	for (auto& currentOperand : this->operands)
	{
		if (currentOperand == nullptr)
		{
			this->failureCause = EquationComputationFailureCause_t::nullOperand;
			doCompute = false;
			break;
		}
		else
		{
			auto currentOperandValue = currentOperand->getInitialValue();
			if (currentOperandValue.getSize() == 0)
			{
				this->failureCause = EquationComputationFailureCause_t::incompleteOperand;
				doCompute = false;
				break;
			}
		}
	}

	// Check for size mismatchs between operands
	if (doCompute == true)
	{
		if ( (this->operatorType != OperatorType_t::concatOp)  &&
		     (this->operatorType != OperatorType_t::extractOp) &&
		     (this->operatorType != OperatorType_t::notOp)     &&
		     (this->operatorType != OperatorType_t::identity)  )
		{
			uint operandsSize = 0;
			for (auto& currentOperand : this->operands)
			{
				auto currentOperandValue = currentOperand->getInitialValue();
				auto currentOperandSize = currentOperandValue.getSize();
				if ( (operandsSize != 0) && (currentOperandSize != operandsSize) )
				{
					this->failureCause = EquationComputationFailureCause_t::sizeMismatch;
					doCompute = false;
					break;
				}
				else if (operandsSize == 0)
				{
					operandsSize = currentOperandSize;
				}
			}
		}
	}

	// Check ranges for extract operand
	if (doCompute == true)
	{
		if (this->operatorType == OperatorType_t::extractOp)
		{
			if (this->rangeL == -1)
			{
				this->failureCause = EquationComputationFailureCause_t::missingParameter;
				doCompute = false;
			}
			else // (this->rangeL != -1)
			{
				if (this->rangeR != -1)
				{
					if (this->rangeL < this->rangeR)
					{
						this->failureCause = EquationComputationFailureCause_t::incorrectParameter;
						doCompute = false;
					}
				}

				auto operand = this->getOperand(0);
				auto operandValue = operand->getInitialValue();
				auto operandSize = operandValue.getSize();
				if ((uint)rangeL >= operandSize)
				{
					this->failureCause = EquationComputationFailureCause_t::incorrectParameter;
					doCompute = false;
				}
			}
		}
	}

	LogicValue previousValue = this->initialValue;
	if (doCompute == true)
	{
		this->initialValue = this->computeValue(CurrentOrInitial_t::initial);
	}
	else
	{
		this->initialValue = LogicValue::getNullValue();
	}

	if (previousValue != this->initialValue)
	{
		emit this->equationInitialValueChangedEvent();
	}
}

void Equation::operandInvalidatedEventHandler()
{
	if (this->operatorType != OperatorType_t::identity)
	{
		for (int operandNumber = 0 ; operandNumber < this->operands.count() ; operandNumber++)
		{
			auto operand = this->getOperand(operandNumber);
			if (operand == nullptr) continue;

			if (operand->getSource() == OperandSource_t::variable)
			{
				auto variableId = operand->getVariableId();
				if (variableId == nullId)
				{
					this->clearOperand(operandNumber);
					break;
				}
			}
		}

		this->computeInitialValue();
		emit this->equationTextChangedEvent();
	}
	else // (this->operatorType == OperatorType_t::identity)
	{
		// Outside an EquationEditor, identity operands are only used at the equation root
		// to hold variables and constants. As operands can only be invalidated as the
		// result of a variable deletion, we know we are outside an EquationEditor.
		// We thus have to transmit the event to the machine component holding the Equation.
		emit this->equationInvalidatedEvent();
	}
}

void Equation::setOperand(uint i, shared_ptr<Operand> newOperand)
{
	// Do not allow placing an operand outside defined range
	if (i >= this->getOperandCount()) return;


	auto previousOperand = this->operands.at(i);
	if (previousOperand != nullptr)
	{
		// In case operand still has a valid pointer elsewere
		disconnect(previousOperand.get(), &Operand::operandTextChangedEvent, this, &Equation::equationTextChangedEvent);

		disconnect(previousOperand.get(), &Operand::operandInitialValueChangedEvent, this, &Equation::computeInitialValue);
		disconnect(previousOperand.get(), &Operand::operandCurrentValueChangedEvent, this, &Equation::computeCurrentValue);
		disconnect(previousOperand.get(), &Operand::operandInvalidatedEvent,         this, &Equation::operandInvalidatedEventHandler);

		// Clean operand
		this->operands[i] = nullptr;
	}

	if (newOperand != nullptr)
	{
		connect(newOperand.get(), &Operand::operandTextChangedEvent, this, &Equation::equationTextChangedEvent);

		connect(newOperand.get(), &Operand::operandInitialValueChangedEvent, this, &Equation::computeInitialValue);
		connect(newOperand.get(), &Operand::operandCurrentValueChangedEvent, this, &Equation::computeCurrentValue);
		connect(newOperand.get(), &Operand::operandInvalidatedEvent,         this, &Equation::operandInvalidatedEventHandler);

		// Assign operand
		this->operands[i] = newOperand;
	}

	this->computeInitialValue();
	emit this->equationTextChangedEvent();

	// As constants current value never change, their only chance of being evaluated is now
	if ( (newOperand != nullptr) && (newOperand->getSource() == OperandSource_t::constant) )
	{
		this->computeCurrentValue();
	}
}

LogicValue Equation::computeValue(CurrentOrInitial_t currentOrInitial)
{
	this->failureCause = EquationComputationFailureCause_t::nofail;

	LogicValue computedValue;
	switch (this->operatorType)
	{
	case OperatorType_t::notOp:
		if (currentOrInitial == CurrentOrInitial_t::current)
		{
			computedValue = ! ( this->operands[0]->getCurrentValue() );
		}
		else
		{
			computedValue = ! ( this->operands[0]->getInitialValue() );
		}
		break;
	case OperatorType_t::identity:
		if (currentOrInitial == CurrentOrInitial_t::current)
		{
			computedValue = this->operands[0]->getCurrentValue();
		}
		else
		{
			computedValue = this->operands[0]->getInitialValue();
		}
		break;
	case OperatorType_t::equalOp:
	{
		LogicValue oneBitResult(1);
		if (currentOrInitial == CurrentOrInitial_t::current)
		{
			oneBitResult[0] = ((this->operands[0]->getCurrentValue() == this->operands[1]->getCurrentValue()));
		}
		else
		{
			oneBitResult[0] = ((this->operands[0]->getInitialValue() == this->operands[1]->getInitialValue()));
		}

		computedValue = oneBitResult;
	}
	break;
	case OperatorType_t::diffOp:
	{
		LogicValue oneBitResult(1);
		if (currentOrInitial == CurrentOrInitial_t::current)
		{
			oneBitResult[0] = ((this->operands[0]->getCurrentValue() != this->operands[1]->getCurrentValue()));
		}
		else
		{
			oneBitResult[0] = ((this->operands[0]->getInitialValue() != this->operands[1]->getInitialValue()));
		}

		computedValue = oneBitResult;
	}
	break;
	case OperatorType_t::extractOp:
		if (this->rangeR != -1)
		{
			int range = this->rangeL - this->rangeR + 1;
			LogicValue subVector(range);
			LogicValue originalValue;
			if (currentOrInitial == CurrentOrInitial_t::current)
			{
				originalValue = this->operands[0]->getCurrentValue();
			}
			else
			{
				originalValue = this->operands[0]->getInitialValue();
			}

			for (int i = 0 ; i < range ; i++)
			{
				subVector[i] = originalValue[this->rangeR + i];
			}

			computedValue = subVector;
		}
		else
		{
			auto operand = this->operands[0];
			LogicValue operandValue;
			if (currentOrInitial == CurrentOrInitial_t::current)
			{
				operandValue = operand->getCurrentValue();
			}
			else
			{
				operandValue = operand->getInitialValue();
			}

			LogicValue result(1);
			result[0] = operandValue[rangeL];
			computedValue = result;
		}
		break;
	case OperatorType_t::concatOp:
	{
		int sizeCount = 0;
		for (auto& currentOperand : this->operands)
		{
			LogicValue currentOperandValue;
			if (currentOrInitial == CurrentOrInitial_t::current)
			{
				currentOperandValue = currentOperand->getCurrentValue();
			}
			else
			{
				currentOperandValue = currentOperand->getInitialValue();
			}

			sizeCount += currentOperandValue.getSize();
		}

		LogicValue concatVector(sizeCount);

		int currentBit = sizeCount - 1;
		for (auto& currentOperand : this->operands)
		{
			LogicValue currentOperandValue;
			if (currentOrInitial == CurrentOrInitial_t::current)
			{
				currentOperandValue = currentOperand->getCurrentValue();
			}
			else
			{
				currentOperandValue = currentOperand->getInitialValue();
			}

			for (int i = currentOperandValue.getSize()-1 ; i >= 0 ; i--)
			{
				if (currentOrInitial == CurrentOrInitial_t::current)
				{
					concatVector[currentBit] = currentOperand->getCurrentValue()[i];
				}
				else
				{
					concatVector[currentBit] = currentOperand->getInitialValue()[i];
				}
				currentBit--;
			}
		}

		computedValue = concatVector;
	}
	break;
	case OperatorType_t::andOp:
	case OperatorType_t::nandOp:
	{
		auto operand = this->getOperand(0);
		uint operandsSize;

		if (currentOrInitial == CurrentOrInitial_t::current)
		{
			operandsSize = operand->getCurrentValue().getSize();
		}
		else
		{
			operandsSize = operand->getInitialValue().getSize();
		}

		LogicValue partialResult(operandsSize, true);
		for (auto& operand : this->operands)
		{
			if (currentOrInitial == CurrentOrInitial_t::current)
			{
				partialResult &= operand->getCurrentValue();
			}
			else
			{
				partialResult &= operand->getInitialValue();
			}
		}

		if (this->isInverted())
			partialResult = !partialResult;

		computedValue = partialResult;
	}
	break;

	case OperatorType_t::orOp:
	case OperatorType_t::norOp:
	{
		auto operand = this->getOperand(0);
		uint operandsSize;

		if (currentOrInitial == CurrentOrInitial_t::current)
		{
			operandsSize = operand->getCurrentValue().getSize();
		}
		else
		{
			operandsSize = operand->getInitialValue().getSize();
		}

		LogicValue partialResult(operandsSize);
		for (auto& operand : this->operands)
		{
			if (currentOrInitial == CurrentOrInitial_t::current)
			{
				partialResult |= operand->getCurrentValue();
			}
			else
			{
				partialResult |= operand->getInitialValue();
			}
		}

		if (this->isInverted())
			partialResult = !partialResult;

		computedValue = partialResult;
	}
	break;

	case OperatorType_t::xorOp:
	case OperatorType_t::xnorOp:
	{
		auto operand = this->getOperand(0);
		uint operandsSize;

		if (currentOrInitial == CurrentOrInitial_t::current)
		{
			operandsSize = operand->getCurrentValue().getSize();
		}
		else
		{
			operandsSize = operand->getInitialValue().getSize();
		}

		LogicValue partialResult(operandsSize);
		for (auto& operand : this->operands)
		{
			if (currentOrInitial == CurrentOrInitial_t::current)
			{
				partialResult ^= operand->getCurrentValue();
			}
			else
			{
				partialResult ^= operand->getInitialValue();
			}
		}

		if (this->isInverted())
			partialResult = !partialResult;

		computedValue = partialResult;
	}
	break;
	}

	return computedValue;
}
