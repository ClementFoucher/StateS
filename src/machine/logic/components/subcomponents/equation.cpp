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
#include "equation.h"

// Qt
#include <QSet>

// StateS
#include "operand.h"


/**
 * @brief Equation::Equation
 * @param operatorType Operator of the equation.
 * @param operandCount Number of operands. This value is ignored
 * for operators that have a fixed-size operand count.
 * Omitting this value for a variable-size operator results in
 * an equation with 2 operands for variable operand functions.
 */
Equation::Equation(Operator_t operatorType, int operandCount)
{
	this->operatorType = operatorType;
	this->failureCause = ComputationFailureCause_t::nullOperand;
	this->warning      = ComputationWarning_t::noWarning;

	// Compute operand count
	uint actualOperandCount;
	switch(this->operatorType)
	{
	case Operator_t::notOp:
	case Operator_t::identity:
	case Operator_t::extractOp:
		actualOperandCount = 1;
		break;
	case Operator_t::equalOp:
	case Operator_t::diffOp:
		actualOperandCount = 2;
		break;
	case Operator_t::andOp:
	case Operator_t::orOp:
	case Operator_t::xorOp:
	case Operator_t::nandOp:
	case Operator_t::norOp:
	case Operator_t::xnorOp:
	case Operator_t::concatOp:
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
	auto clonedEquation = make_shared<Equation>(this->operatorType, this->operands.count());

	for (uint i = 0 ; i < this->operands.count() ; i++)
	{
		auto operand = this->operands.at(i);
		if (operand != nullptr)
		{
			clonedEquation->setOperand(i, operand->clone());
		}
	}

	if (this->operatorType == Operator_t::extractOp)
	{
		clonedEquation->setRange(this->rangeL, this->rangeR);
	}

	return clonedEquation;
}

bool Equation::isValid() const
{
	return !this->initialValue.isNull();
}

MachineValue::Type_t Equation::getType() const
{
	return this->initialValue.getType();
}

MachineValue Equation::getInitialValue() const
{
	return this->initialValue;
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
		if (this->isValid() == false)
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
	if (this->isInverted() == true)
	{
		text += '/';
	}

	if (operandCount > 1)
	{
		text += "( ";
	}

	for (uint i = 0 ; i < operandCount ; i++)
	{
		auto operand = this->operands[i];
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
			case Operator_t::andOp:
			case Operator_t::nandOp:
				text += " • ";
				break;
			case Operator_t::orOp:
			case Operator_t::norOp:
				text += " + ";
				break;
			case Operator_t::xorOp:
			case Operator_t::xnorOp:
				text += " ⊕ ";
				break;
			case Operator_t::equalOp:
				text += " = ";
				break;
			case Operator_t::diffOp:
				text += " ≠ ";
				break;
			case Operator_t::concatOp:
				text += " : ";
				break;
			case Operator_t::notOp:
			case Operator_t::identity:
			case Operator_t::extractOp:
				break;
			}
		}
	}

	if (this->operatorType == Operator_t::extractOp)
	{
		text += "[";

		if (this->rangeL != -1)
		{
			text += QString::number(this->rangeL);
		}
		else
		{
			text += "…";
		}

		if (this->rangeR != -1)
		{
			text += ".." + QString::number(this->rangeR);
		}

		text += "]";
	}

	if (operandCount > 1)
	{
		text += " )";
	}

	if (raw == false)
	{
		text += "</span>";
	}

	return text;
}

Equation::ComputationFailureCause_t Equation::getComputationFailureCause() const
{
	return this->failureCause;
}

Equation::ComputationWarning_t Equation::getComputationWarning() const
{
	return this->warning;
}

void Equation::setOperator(Operator_t newOperator)
{
	if (newOperator == this->operatorType) return;


	// Update operator
	this->operatorType = newOperator;

	// Make sure operand count is correct for the new operator
	switch(this->operatorType)
	{
	case Operator_t::notOp:
	case Operator_t::identity:
	case Operator_t::extractOp:
		while (this->operands.size() > 1)
		{
			this->operands.removeLast();
		}
		break;
	case Operator_t::equalOp:
	case Operator_t::diffOp:
		while (this->operands.size() < 2)
		{
			this->operands.append(nullptr);
		}
		while (this->operands.size() > 2)
		{
			this->operands.removeLast();
		}
		break;
	case Operator_t::andOp:
	case Operator_t::orOp:
	case Operator_t::xorOp:
	case Operator_t::nandOp:
	case Operator_t::norOp:
	case Operator_t::xnorOp:
	case Operator_t::concatOp:
		while (this->operands.size() < 2)
		{
			this->operands.append(nullptr);
		}
		break;
	}

	// Compute new value
	this->checkAndComputeInitialValue();

	emit this->equationTextChangedEvent();
}

Equation::Operator_t Equation::getOperator() const
{
	return this->operatorType;
}

QSet<ComponentId> Equation::getVariablesIdsSet() const
{
	QSet<ComponentId> variablesIds;

	for (const auto& operand : this->operands)
	{
		if (operand == nullptr) continue;


		if (operand->getSource() == Operand::Source_t::equation)
		{
			auto equation = operand->getEquation();
			if (equation == nullptr) continue;


			variablesIds += equation->getVariablesIdsSet();
		}
		else if (operand->getSource() == Operand::Source_t::variable)
		{
			variablesIds.insert(operand->getVariableId());
		}
	}

	return variablesIds;
}

bool Equation::isInverted() const
{
	switch (this->operatorType)
	{
	case Operator_t::notOp:
	case Operator_t::nandOp:
	case Operator_t::norOp:
	case Operator_t::xnorOp:
		return true;
		break;
	case Operator_t::identity:
	case Operator_t::andOp:
	case Operator_t::orOp:
	case Operator_t::xorOp:
	case Operator_t::equalOp:
	case Operator_t::diffOp:
	case Operator_t::extractOp:
	case Operator_t::concatOp:
		return false;
		break;
	}
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

void Equation::setOperand(uint i, ComponentId newOperand)
{
	auto operand = make_shared<Operand>(newOperand);
	this->setOperand(i, operand);
}

void Equation::setOperand(uint i, shared_ptr<Equation> newOperand)
{
	auto operand = make_shared<Operand>(newOperand);
	this->setOperand(i, operand);
}

void Equation::setOperand(uint i, MachineValue newOperand)
{
	auto operand = make_shared<Operand>(newOperand);
	this->setOperand(i, operand);
}

void Equation::setOperand(uint i, shared_ptr<Variable> newOperand)
{
	auto operand = make_shared<Operand>(newOperand);
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
	case Operator_t::notOp:
	case Operator_t::identity:
	case Operator_t::extractOp:
		return 1;
		break;
	case Operator_t::equalOp:
	case Operator_t::diffOp:
		return 2;
		break;
	case Operator_t::andOp:
	case Operator_t::orOp:
	case Operator_t::xorOp:
	case Operator_t::nandOp:
	case Operator_t::norOp:
	case Operator_t::xnorOp:
	case Operator_t::concatOp:
		return this->operands.size();
		break;
	}
}

void Equation::increaseOperandCount()
{
	bool doIncrease = false;

	switch (this->operatorType)
	{
	case Operator_t::andOp:
	case Operator_t::orOp:
	case Operator_t::xorOp:
	case Operator_t::nandOp:
	case Operator_t::norOp:
	case Operator_t::xnorOp:
	case Operator_t::concatOp:
		doIncrease = true;
		break;
	case Operator_t::extractOp:
	case Operator_t::notOp:
	case Operator_t::identity:
	case Operator_t::equalOp:
	case Operator_t::diffOp:
		// Do not allow modifying operand count for these fixed-size operators
		break;
	}

	if (doIncrease == true)
	{
		this->operands.append(nullptr);
		this->checkAndComputeInitialValue();
		emit this->equationTextChangedEvent();
	}
}

void Equation::decreaseOperandCount()
{
	bool doDecrease = false;

	switch (this->operatorType)
	{
	case Operator_t::andOp:
	case Operator_t::orOp:
	case Operator_t::xorOp:
	case Operator_t::nandOp:
	case Operator_t::norOp:
	case Operator_t::xnorOp:
	case Operator_t::concatOp:
		if (this->getOperandCount() > 2)
		{
			doDecrease = true;
		}
		break;
	case Operator_t::extractOp:
	case Operator_t::notOp:
	case Operator_t::identity:
	case Operator_t::equalOp:
	case Operator_t::diffOp:
		// Do not allow modifying operand count for these fixed-size operators
		break;
	}

	if (doDecrease == true)
	{
		this->operands.removeLast();
		this->checkAndComputeInitialValue();
		emit this->equationTextChangedEvent();
	}
}

void Equation::setRange(int rangeL, int rangeR)
{
	if (this->operatorType == Operator_t::extractOp)
	{
		this->rangeL = rangeL;
		this->rangeR = rangeR;

		this->checkAndComputeInitialValue();
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


		if (operand->getSource() == Operand::Source_t::equation)
		{
			auto equation = operand->getEquation();
			if (equation != nullptr)
			{
				equation->doFullStackRecomputation();
			}
		}
	}

	this->checkAndComputeInitialValue();
}

void Equation::checkAndComputeInitialValue()
{
	this->checkForErrors();

	auto previousValue = this->initialValue;
	if (this->failureCause != ComputationFailureCause_t::nofail)
	{
		this->initialValue = MachineValue{};
	}
	else
	{
		this->initialValue = this->computeInitialValue();
	}

	if (previousValue != this->initialValue)
	{
		emit this->equationInitialValueChangedEvent();
	}
}

void Equation::operandInvalidatedEventHandler()
{
	if (this->operatorType != Operator_t::identity)
	{
		for (int operandNumber = 0 ; operandNumber < this->operands.count() ; operandNumber++)
		{
			auto operand = this->operands[operandNumber];
			if (operand == nullptr) continue;


			if (operand->getSource() == Operand::Source_t::variable)
			{
				auto variableId = operand->getVariableId();
				if (variableId == nullId)
				{
					this->clearOperand(operandNumber);
					break;
				}
			}
		}

		this->checkAndComputeInitialValue();

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

		disconnect(previousOperand.get(), &Operand::operandInitialValueChangedEvent, this, &Equation::checkAndComputeInitialValue);
		disconnect(previousOperand.get(), &Operand::operandInvalidatedEvent,         this, &Equation::operandInvalidatedEventHandler);

		// Clean operand
		this->operands[i] = nullptr;
	}

	if (newOperand != nullptr)
	{
		connect(newOperand.get(), &Operand::operandTextChangedEvent, this, &Equation::equationTextChangedEvent);

		connect(newOperand.get(), &Operand::operandInitialValueChangedEvent, this, &Equation::checkAndComputeInitialValue);
		connect(newOperand.get(), &Operand::operandInvalidatedEvent,         this, &Equation::operandInvalidatedEventHandler);

		// Assign operand
		this->operands[i] = newOperand;
	}

	this->checkAndComputeInitialValue();
	emit this->equationTextChangedEvent();
}

void Equation::checkForErrors()
{
	// PRE: operand count is correct wrt. operator (enforced elsewhere in the class)

	this->warning      = ComputationWarning_t::noWarning;
	this->failureCause = ComputationFailureCause_t::nofail;

	// Check for null or invalid operands
	for (auto& currentOperand : this->operands)
	{
		if (currentOperand == nullptr)
		{
			this->failureCause = ComputationFailureCause_t::nullOperand;
			return;
		}
		else
		{
			if (currentOperand->getType() == MachineValue::Type_t::nullType)
			{
				this->failureCause = ComputationFailureCause_t::invalidOperandValue;
				return;
			}
		}
	}
	// POST: all operands are valid

	// Check operands types and sizes
	switch (this->operatorType)
	{
	case Operator_t::extractOp:
	{
		if (this->operands[0]->getType() != MachineValue::Type_t::bitVector)
		{
			this->failureCause = ComputationFailureCause_t::incorrectOperandType;
			return;
		}
		// POST: operand 0 is of type Bit Vector
		break;
	}
	case Operator_t::concatOp:
	{
		for (auto& currentOperand : this->operands)
		{
			auto currentOperandType = currentOperand->getType();
			if ( (currentOperandType != MachineValue::Type_t::boolean) &&
			     (currentOperandType != MachineValue::Type_t::bitVector)
			   )
			{
				this->failureCause = ComputationFailureCause_t::incorrectOperandType;
				return;
			}
		}
		// POST: all operands are either Boolean or Bit Vector
		break;
	}
	case Operator_t::andOp:
	case Operator_t::nandOp:
	case Operator_t::orOp:
	case Operator_t::norOp:
	case Operator_t::xorOp:
	case Operator_t::xnorOp:
	{
		auto operandsType = MachineValue::Type_t::nullType;
		uint bitVectorOperandsSize = 0;

		// First check for unauthorized types
		for (auto& currentOperand : this->operands)
		{
			auto currentOperandType = currentOperand->getType();

			if ( (currentOperandType != MachineValue::Type_t::boolean) &&
			     (currentOperandType != MachineValue::Type_t::bitVector)
			   )
			{
				this->failureCause = ComputationFailureCause_t::incorrectOperandType;
				return;
			}
			else if (operandsType == MachineValue::Type_t::nullType)
			{
				// The first operand is used as reference for the expected type...
				operandsType = currentOperandType;

				// ... and size for Bit Vector operands
				if (operandsType == MachineValue::Type_t::bitVector)
				{
					bitVectorOperandsSize = currentOperand->getInitialValue().getBitVectorValue().getSize();
				}
			}
		}
		// POST: all operands are either Boolean or Bit Vector

		// Then check for types and sizes mismatchs
		for (auto& currentOperand : this->operands)
		{
			auto currentOperandType = currentOperand->getType();

			if (currentOperandType != operandsType)
			{
				this->failureCause = ComputationFailureCause_t::operandsTypesMismatch;
				return;
			}
			else if ( (operandsType == MachineValue::Type_t::bitVector) &&
			          (currentOperand->getInitialValue().getBitVectorValue().getSize() != bitVectorOperandsSize)
			        )
			{
				this->failureCause = ComputationFailureCause_t::operandsSizesMismatch;
				return;
			}
		}
		// POST: all operands have the same type
		// POST: Bit Vector operands have the same size

		break;
	}
	case Operator_t::equalOp:
	case Operator_t::diffOp:
	{
		auto operand0Type = this->operands[0]->getType();
		auto operand1Type = this->operands[1]->getType();
		if (operand0Type != operand1Type)
		{
			this->warning = ComputationWarning_t::differentTypeComparison;
		}
		else if (operand0Type == MachineValue::Type_t::bitVector)
		{
			auto operand0Size = this->operands[0]->getInitialValue().getBitVectorValue().getSize();
			auto operand1Size = this->operands[1]->getInitialValue().getBitVectorValue().getSize();
			if (operand0Size != operand1Size)
			{
				this->warning = ComputationWarning_t::differentSizeComparison;
			}
		}
		break;
	}
	case Operator_t::notOp:
	{
		auto operand0Type = this->operands[0]->getType();
		if ( (operand0Type != MachineValue::Type_t::boolean) &&
		     (operand0Type != MachineValue::Type_t::bitVector)
		   )
		{
			this->failureCause = ComputationFailureCause_t::incorrectOperandType;
			return;
		}
		break;
	}
	case Operator_t::identity:
		// Nothing to check here
		break;
	}

	// Check ranges for extract operand
	if (this->operatorType == Operator_t::extractOp)
	{
		// PRE: operand 0 is of type Bit Vector

		if (this->rangeL == -1)
		{
			this->failureCause = ComputationFailureCause_t::missingParameter;
			return;
		}
		else // (this->rangeL != -1)
		{
			if (this->rangeR != -1)
			{
				if (this->rangeL < this->rangeR)
				{
					this->failureCause = ComputationFailureCause_t::incorrectParameterValue;
					return;
				}
			}

			if (static_cast<uint>(rangeL) >= this->operands[0]->getInitialValue().getBitVectorValue().getSize())
			{
				this->failureCause = ComputationFailureCause_t::incorrectParameterValue;
				return;
			}
		}
	}
	// POST: right range is -1 and left range is < to Bit Vector size and > to 0
	//       OR right range is >= to 0 and <= left range AND left range is < to Bit Vector size
}

MachineValue Equation::computeInitialValue()
{
	MachineValue computedValue{};
	switch (this->operatorType)
	{
	case Operator_t::notOp:
	case Operator_t::identity:
		// PRE: there is one operand
		// PRE: the operand is valid

		computedValue = this->operands[0]->getInitialValue();
		break;
	case Operator_t::equalOp:
	{
		// PRE: there are two operands
		// PRE: the two operands are valid

		computedValue = BooleanValue{(this->operands[0]->getInitialValue() == this->operands[1]->getInitialValue())};
		break;
	}
	case Operator_t::diffOp:
	{
		// PRE: there are two operands
		// PRE: the two operands are valid

		computedValue = BooleanValue{(this->operands[0]->getInitialValue() != this->operands[1]->getInitialValue())};
		break;
	}
	case Operator_t::extractOp:
	{
		// PRE: there is one operand
		// PRE: the operand is valid
		// PRE: the operand is of type Bit Vector
		// PRE: right range is -1 and left range is < to Bit Vector size and > to 0
		//      OR right range is >= to 0 and <= left range AND left range is < to Bit Vector size

		auto operandValue = this->operands[0]->getInitialValue().getBitVectorValue();
		if (this->rangeR != -1)
		{
			int range = this->rangeL - this->rangeR + 1;
			auto subVector = BitVectorValue::allZeros(range);

			for (int i = 0 ; i < range ; i++)
			{
				subVector.setBit(i, operandValue[this->rangeR + i]);
			}

			computedValue = subVector;
		}
		else
		{
			computedValue = BooleanValue{operandValue[rangeL]};
		}
		break;
	}
	case Operator_t::concatOp:
	{
		// PRE: there are two or more operands
		// PRE: the operands are all valid
		// PRE: all operands are either Boolean or Bit Vector

		int sizeCount = 0;
		for (auto& currentOperand : this->operands)
		{
			if (currentOperand->getType() == MachineValue::Type_t::bitVector)
			{
				sizeCount += currentOperand->getInitialValue().getBitVectorValue().getSize();
			}
			else // (currentOperandValue.getValueType() == MachineValue::ValueType_t::boolean)
			{
				sizeCount++;
			}
		}

		auto concatVector = BitVectorValue::allZeros(sizeCount);

		int currentBit = sizeCount - 1;
		for (auto& currentOperand : this->operands)
		{
			if (currentOperand->getType() == MachineValue::Type_t::bitVector)
			{
				auto currentOperandBitVectorValue = currentOperand->getInitialValue().getBitVectorValue();
				for (int i = static_cast<int>(currentOperandBitVectorValue.getSize())-1 ; i >= 0 ; i--)
				{
					concatVector.setBit(currentBit, currentOperandBitVectorValue[i]);
					currentBit--;
				}
			}
			else // (currentOperandValue.getValueType() == MachineValue::ValueType_t::boolean)
			{
				auto currentOperandBooleanValue = currentOperand->getInitialValue().getBooleanValue();
				concatVector.setBit(currentBit, currentOperandBooleanValue);
				currentBit--;
			}
		}

		computedValue = concatVector;
		break;
	}
	case Operator_t::andOp:
	case Operator_t::nandOp:
	case Operator_t::orOp:
	case Operator_t::norOp:
	case Operator_t::xorOp:
	case Operator_t::xnorOp:
	{
		// PRE: there are two or more operands
		// PRE: the operands are all valid
		// PRE: operands are either Boolean or Bit Vector
		// PRE: all operands have the same type
		// PRE: Bit Vector operands have the same size

		auto firstOperand = this->operands[0];
		if (firstOperand->getType() == MachineValue::Type_t::bitVector)
		{
			auto firstOperandBitVectorValue = firstOperand->getInitialValue().getBitVectorValue();
			uint firstOperandsSize = firstOperandBitVectorValue.getSize();

			BitVectorValue partialResult{};

			switch (this->operatorType)
			{
			case Operator_t::andOp:
			case Operator_t::nandOp:
				partialResult = BitVectorValue::allOnes(firstOperandsSize);
				break;
			case Operator_t::orOp:
			case Operator_t::norOp:
			case Operator_t::xorOp:
			case Operator_t::xnorOp:
				partialResult = BitVectorValue::allZeros(firstOperandsSize);
				break;
			default:
				break;
			}

			for (auto& operand : this->operands)
			{
				auto currentOperandValue = operand->getInitialValue();
				auto bitVectorOperandValue = currentOperandValue.getBitVectorValue();

				switch (this->operatorType)
				{
				case Operator_t::andOp:
				case Operator_t::nandOp:
					partialResult &= bitVectorOperandValue;
					break;
				case Operator_t::orOp:
				case Operator_t::norOp:
					partialResult |= bitVectorOperandValue;
					break;
				case Operator_t::xorOp:
				case Operator_t::xnorOp:
					partialResult ^= bitVectorOperandValue;
					break;
				default:
					break;
				}
			}

			computedValue = partialResult;
		}
		else // (firstOperand->getType() == MachineValue::ValueType_t::boolean)
		{
			BooleanValue partialResult{};

			switch (this->operatorType)
			{
			case Operator_t::andOp:
			case Operator_t::nandOp:
				partialResult = BooleanValue::trueValue();
				break;
			case Operator_t::orOp:
			case Operator_t::norOp:
			case Operator_t::xorOp:
			case Operator_t::xnorOp:
				partialResult = BooleanValue::falseValue();
				break;
			default:
				break;
			}

			for (auto& operand : this->operands)
			{
				auto currentOperandValue = operand->getInitialValue();
				auto booleanOperandValue = currentOperandValue.getBooleanValue();

				switch (this->operatorType)
				{
				case Operator_t::andOp:
				case Operator_t::nandOp:
					partialResult &= booleanOperandValue;
					break;
				case Operator_t::orOp:
				case Operator_t::norOp:
					partialResult |= booleanOperandValue;
					break;
				case Operator_t::xorOp:
				case Operator_t::xnorOp:
					partialResult ^= booleanOperandValue;
					break;
				default:
					break;
				}
			}

			computedValue = partialResult;
		}

		break;
	}
	}

	if (this->isInverted() == true)
	{
		// PRE: the result is either Boolean or Bit Vector
		if (computedValue.getType() == MachineValue::Type_t::bitVector)
		{
			computedValue = ~(computedValue.getBitVectorValue());
		}
		else // (computedValue.getType() == MachineValue::ValueType_t::boolean)
		{
			computedValue = !(computedValue.getBooleanValue());
		}
	}

	return computedValue;
}
