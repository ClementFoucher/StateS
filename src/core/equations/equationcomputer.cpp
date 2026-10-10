/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "equationcomputer.h"


MachineValue EquationComputer::computeValue(Equation::Operator_t operator_, const std::vector<MachineValue> operands, int rangeL, int rangeR)
{
	MachineValue computedValue{};
	switch (operator_)
	{
	case Equation::Operator_t::notOp:
	case Equation::Operator_t::identity:
		// PRE: there is one operand
		// PRE: the operand is valid

		computedValue = operands[0];
		break;
	case Equation::Operator_t::equalOp:
		// PRE: there are two operands
		// PRE: the two operands are valid

		computedValue = BooleanValue{(operands[0] == operands[1])};
		break;
	case Equation::Operator_t::diffOp:
		// PRE: there are two operands
		// PRE: the two operands are valid

		computedValue = BooleanValue{(operands[0] != operands[1])};
		break;
	case Equation::Operator_t::extractOp:
	{
		// PRE: there is one operand
		// PRE: the operand is valid
		// PRE: the operand is of type Bit Vector
		// PRE: right range is -1 and left range is < to Bit Vector size and > to 0
		//      OR right range is >= to 0 and <= left range AND left range is < to Bit Vector size

		auto operandBitVectorValue = operands[0].getBitVectorValue();
		if (rangeR != -1)
		{
			int range = rangeL - rangeR + 1;
			auto subVector = BitVectorValue::allZeros(range);

			for (int i = 0 ; i < range ; i++)
			{
				subVector.setBit(i, operandBitVectorValue[rangeR + i]);
			}

			computedValue = subVector;
		}
		else
		{
			computedValue = BooleanValue{operandBitVectorValue[rangeL]};
		}
		break;
	}
	case Equation::Operator_t::concatOp:
	{
		// PRE: there are two or more operands
		// PRE: the operands are all valid
		// PRE: all operands are either Boolean or Bit Vector

		int sizeCount = 0;
		for (auto& operand : operands)
		{
			if (operand.getType() == MachineValue::Type_t::bitVector)
			{
				sizeCount += operand.getBitVectorValue().getSize();
			}
			else // (operand.getType() == MachineValue::Type_t::boolean)
			{
				sizeCount++;
			}
		}

		auto concatVector = BitVectorValue::allZeros(sizeCount);

		int currentBit = sizeCount - 1;
		for (auto& operand : operands)
		{
			if (operand.getType() == MachineValue::Type_t::bitVector)
			{
				auto operandBitVectorValue = operand.getBitVectorValue();
				for (int i = static_cast<int>(operandBitVectorValue.getSize())-1 ; i >= 0 ; i--)
				{
					concatVector.setBit(currentBit, operandBitVectorValue[i]);
					currentBit--;
				}
			}
			else // (operand.getType() == MachineValue::Type_t::boolean)
			{
				auto currentOperandBooleanValue = operand.getBooleanValue();
				concatVector.setBit(currentBit, currentOperandBooleanValue);
				currentBit--;
			}
		}

		computedValue = concatVector;
		break;
	}
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::xnorOp:
	{
		// PRE: there are two or more operands
		// PRE: the operands are all valid
		// PRE: operands are either Boolean or Bit Vector
		// PRE: all operands have the same type
		// PRE: Bit Vector operands have the same size

		auto firstOperand = operands[0];
		if (firstOperand.getType() == MachineValue::Type_t::bitVector)
		{
			auto firstOperandBitVectorValue = firstOperand.getBitVectorValue();
			uint firstOperandsSize = firstOperandBitVectorValue.getSize();

			BitVectorValue partialResult{};

			switch (operator_)
			{
			case Equation::Operator_t::andOp:
			case Equation::Operator_t::nandOp:
				partialResult = BitVectorValue::allOnes(firstOperandsSize);
				break;
			case Equation::Operator_t::orOp:
			case Equation::Operator_t::norOp:
			case Equation::Operator_t::xorOp:
			case Equation::Operator_t::xnorOp:
				partialResult = BitVectorValue::allZeros(firstOperandsSize);
				break;
			default:
				break;
			}

			for (auto& operandValue : operands)
			{
				auto bitVectorOperandValue = operandValue.getBitVectorValue();

				switch (operator_)
				{
				case Equation::Operator_t::andOp:
				case Equation::Operator_t::nandOp:
					partialResult &= bitVectorOperandValue;
					break;
				case Equation::Operator_t::orOp:
				case Equation::Operator_t::norOp:
					partialResult |= bitVectorOperandValue;
					break;
				case Equation::Operator_t::xorOp:
				case Equation::Operator_t::xnorOp:
					partialResult ^= bitVectorOperandValue;
					break;
				default:
					break;
				}
			}

			computedValue = partialResult;
		}
		else // (firstOperand.getType() == MachineValue::Type_t::boolean)
		{
			BooleanValue partialResult{};

			switch (operator_)
			{
			case Equation::Operator_t::andOp:
			case Equation::Operator_t::nandOp:
				partialResult = BooleanValue::trueValue();
				break;
			case Equation::Operator_t::orOp:
			case Equation::Operator_t::norOp:
			case Equation::Operator_t::xorOp:
			case Equation::Operator_t::xnorOp:
				partialResult = BooleanValue::falseValue();
				break;
			default:
				break;
			}

			for (auto& operandValue : operands)
			{
				auto booleanOperandValue = operandValue.getBooleanValue();

				switch (operator_)
				{
				case Equation::Operator_t::andOp:
				case Equation::Operator_t::nandOp:
					partialResult &= booleanOperandValue;
					break;
				case Equation::Operator_t::orOp:
				case Equation::Operator_t::norOp:
					partialResult |= booleanOperandValue;
					break;
				case Equation::Operator_t::xorOp:
				case Equation::Operator_t::xnorOp:
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

	if (EquationComputer::isInverted(operator_) == true)
	{
		// PRE: the result is either Boolean or Bit Vector
		if (computedValue.getType() == MachineValue::Type_t::bitVector)
		{
			computedValue = ~(computedValue.getBitVectorValue());
		}
		else // (computedValue.getType() == MachineValue::Type_t::boolean)
		{
			computedValue = !(computedValue.getBooleanValue());
		}
	}

	return computedValue;
}

bool EquationComputer::isInverted(Equation::Operator_t op)
{
	switch (op)
	{
	case Equation::Operator_t::notOp:
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xnorOp:
		return true;
		break;
	case Equation::Operator_t::identity:
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::equalOp:
	case Equation::Operator_t::diffOp:
	case Equation::Operator_t::extractOp:
	case Equation::Operator_t::concatOp:
		return false;
		break;
	}
}
