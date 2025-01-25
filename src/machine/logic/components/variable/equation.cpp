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

// Debug
#include <QDebug>
#include <QMetaMethod>

// StateS classes
#include "statestypes.h"
#include "statesexception.h"
#include "exceptiontypes.h"


// Main constructor
/**
 * @brief Equation::Equation
 * @param function
 * @param operandCount Number of operands. Omitting this value results in a default-sized equation with 2 operands for variable operand functions.
 */
Equation::Equation(OperatorType_t function, int operandCount) :
    Variable("<sub>(equation)</sub>")
{
	this->function = function;
	this->currentValue = LogicValue::getNullValue();

	// Operand count affectation
	switch(this->function)
	{
	case OperatorType_t::constant:
		this->allowedOperandCount = 0;

		if ( (operandCount != 0) && (operandCount >= 0) )
		{
			qDebug() << "(Equation:) Warning: Trying to create constant equation with size != 0 (requested size is " << QString::number(operandCount) << ").";
			qDebug() << "Requested size value ignored and set to 0.";
		}
		break;
	case OperatorType_t::extractOp:
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
		this->allowedOperandCount = 1;

		if ( (operandCount != 1) && (operandCount >= 0) )
		{
			qDebug() << "(Equation:) Warning: Trying to create a fixed size 1 equation with size != 1 (requested size is " << QString::number(operandCount) << ").";
			qDebug() << "Requested size value ignored and set to 1.";
		}
		break;
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		this->allowedOperandCount = 2;

		if ( (operandCount != 2) && (operandCount >= 0) )
		{
			qDebug() << "(Equation) Warning: Trying to create a fixed size 2 equation with size != 2 (requested size is " << QString::number(operandCount) << ").";
			qDebug() << "Requested size value ignored and set to 2.";
		}
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
			this->allowedOperandCount = operandCount;
		}
		else if (operandCount < 0)
		{
			this->allowedOperandCount = 2;
		}
		else
		{
			this->allowedOperandCount = 2;

			qDebug() << "(Equation) Warning: Trying to create an equation with size < 2 (requested size is " << QString::number(operandCount) << ").";
			qDebug() << "Requested size value ignored and set to 2.";
		}
	}

	// Default values
	this->rangeL = -1;
	this->rangeR = -1;
	this->constantValue = LogicValue::getNullValue();

	this->signalOperands   = QVector<weak_ptr<Variable>>    (this->allowedOperandCount);
	this->equationOperands = QVector<shared_ptr<Equation>>(this->allowedOperandCount);
}

Equation::Equation(OperatorType_t function, const QVector<shared_ptr<Variable>>& operandList) :
    Equation(function, operandList.count())
{
	for (int i = 0 ; i < operandList.count() ; i++)
	{
		// Do not compute value as done in a mute way
		this->setOperand(i, operandList[i], true);
	}

	// Causes recomputation
	this->computeCurrentValue();
}

shared_ptr<Equation> Equation::clone() const
{
	shared_ptr<Equation> eq = shared_ptr<Equation>(new Equation(this->function, this->getOperands()));

	if (this->function == OperatorType_t::constant)
	{
		eq->setConstantValue(this->constantValue); // Throws StatesException - constantValue is built for signal size - ignored
	}
	else if (this->function == OperatorType_t::extractOp)
	{
		eq->setRange(this->rangeL, this->rangeR);
	}


	return eq;
}

/**
 * @brief getSize returns the apparent size of the equation.
 * 0 means current size is not defined => result will be a null signal.
 * @return
 */
uint Equation::getSize() const
{
	return currentValue.getSize();
}

void Equation::resize(uint) // Throws StatesException
{
	// Equation size is dynamic (or is 1 for eq and diff).
	// Can't be resized.
	throw StatesException("Equation", EquationError_t::resized_requested, "Trying to resize an equation");
}

OperatorType_t Equation::getFunction() const
{
	return function;
}

void Equation::setFunction(OperatorType_t newFunction)
{
	switch(newFunction)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::extractOp:
		// Delete operands beyond one
		while (this->allowedOperandCount > 1)
		{
			// Muted: do not causes recomputation
			decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored
		}
		break;
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		// Exactly two operands needed

		// Delete operands beyond two
		while (this->allowedOperandCount > 2)
		{
			decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored
		}

		// Add operand if not enough
		if (allowedOperandCount == 1)
		{
			increaseOperandCountInternal();
		}

		break;
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		// At least two operands for all other equation types
		if (allowedOperandCount == 1)
		{
			increaseOperandCountInternal();
		}
		break;
	case OperatorType_t::constant:
		// Delete operands
		while (allowedOperandCount > 0)
		{
			decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored
		}
		break;
	}

	this->function = newFunction;

	if (this->function == OperatorType_t::extractOp)
	{
		this->rangeL = -1;
		this->rangeR = -1;
	}

	emit variableStaticConfigurationChangedEvent();

	this->computeCurrentValue();
}

void Equation::setRange(int rangeL, int rangeR)
{
	if (this->function == OperatorType_t::extractOp)
	{
		this->rangeL = rangeL;
		this->rangeR = rangeR;

		emit variableStaticConfigurationChangedEvent();

		this->computeCurrentValue();
	}

}

shared_ptr<Variable> Equation::getOperand(uint i) const // Throws StatesException
{
	if (i < this->allowedOperandCount)
		return this->getOperands().at(i);
	else
		throw StatesException("Equation", EquationError_t::out_of_range_access, "Out of range operand access");
}

/**
 * @brief Equation::setOperand
 * @param i
 * @param newOperand
 * Equation always owns its own Equation operands.
 * All given operands that are equation will be cloned.
 * @return
 */
bool Equation::setOperand(uint i, shared_ptr<Variable> newOperand, bool quiet) // Throws StatesException
{
	// Do not allow placing an operand outside defined range
	if (i >= this->allowedOperandCount)
	{
		throw StatesException("Equation", EquationError_t::out_of_range_access, "Out of range operand access");
	}

	// Only proceed to set if new operand is not the same as the current one.
	// If it is, just return success
	if (this->getOperand(i) == newOperand) // Throws StatesException - Operand count checked - ignored
	{
		return true;
	}
	else
	{
		// Muted: do not causes recomputation
		clearOperand(i, true); // Throws StatesException - Operand count checked - ignored

		if (newOperand != nullptr)
		{
			// Assign operand

			shared_ptr<Equation> newEquationOperand = dynamic_pointer_cast <Equation> (newOperand);
			shared_ptr<Variable> actualNewOperand;

			if (newEquationOperand != nullptr)
			{
				equationOperands[i] = newEquationOperand->clone();
				actualNewOperand = dynamic_pointer_cast<Variable>(equationOperands[i]);
			}
			else
			{
				connect(newOperand.get(), &Variable::variableDeletedEvent, this, &Equation::signalDeletedEventHandler);
				signalOperands[i] = newOperand;
				actualNewOperand = newOperand;
			}


			// Structural changes are propagated
			connect(actualNewOperand.get(), &Variable::variableStaticConfigurationChangedEvent, this, &Variable::variableStaticConfigurationChangedEvent);
			// Local stuff
			connect(actualNewOperand.get(), &Variable::variableStaticConfigurationChangedEvent, this, &Equation::computeCurrentValue);
			connect(actualNewOperand.get(), &Variable::variableDynamicStateChangedEvent,        this, &Equation::computeCurrentValue);
		}

		if (!quiet)
		{
			emit variableStaticConfigurationChangedEvent();

			this->computeCurrentValue();
		}

		return true;
	}
}

void Equation::clearOperand(uint i, bool quiet) // Throws StatesException
{
	// Chek index
	if (i >= this->allowedOperandCount)
	{
		throw StatesException("Equation", EquationError_t::out_of_range_access, "Out of range operand access");
	}

	shared_ptr<Variable> oldOperand = this->getOperand(i); // Throws StatesException - Operand count checked - ignored

	if (oldOperand != nullptr)
	{
		// Signal propagation
		disconnect(oldOperand.get(), &Variable::variableStaticConfigurationChangedEvent, this, &Variable::variableStaticConfigurationChangedEvent);
		// Local stuff
		disconnect(oldOperand.get(), &Variable::variableStaticConfigurationChangedEvent, this, &Equation::computeCurrentValue);
		disconnect(oldOperand.get(), &Variable::variableDynamicStateChangedEvent,        this, &Equation::computeCurrentValue);

		if (oldOperand != nullptr)
		{
			disconnect(oldOperand.get(), &Variable::variableDeletedEvent, this, &Equation::signalDeletedEventHandler);
		}

		equationOperands[i].reset();
		signalOperands[i].reset();

		if (!quiet)
		{
			emit variableStaticConfigurationChangedEvent();

			this->computeCurrentValue();
		}
	}
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

	if (this->function == OperatorType_t::constant)
	{
		text += this->currentValue.toString();
	}
	else
	{
		// Inversion oeprator
		if (this->isInverted())
			text += '/';

		if (this->allowedOperandCount > 1)
			text += "( ";

		for (uint i = 0 ; i < this->allowedOperandCount ; i++)
		{
			shared_ptr<Variable> signalOperand = getOperand(i); // Throws StatesException - Contrained by operand count - ignored
			shared_ptr<Equation> equationOperand = dynamic_pointer_cast<Equation>(signalOperand);

			if (equationOperand != nullptr)
			{
				text += equationOperand->getColoredText(raw);
			}
			else if (signalOperand != nullptr)
			{
				text += signalOperand->getText();
			}
			else
			{
				text += "…";
			}

			// Add operator, except for last operand
			if (i < this->allowedOperandCount - 1)
			{
				switch(function)
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
				case OperatorType_t::constant:
					break;
				}
			}
		}

		if (this->function == OperatorType_t::extractOp)
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

		if (this->allowedOperandCount > 1)
			text += " )";
	}

	if (raw == false)
	{
		text += "</span>";
	}

	return text;
}

void Equation::setConstantValue(const LogicValue& value) // Throws StatesException
{
	if (this->function == OperatorType_t::constant)
	{
		this->constantValue = value;

		this->computeCurrentValue();

		emit Variable::variableStaticConfigurationChangedEvent();
	}
	else
		throw StatesException("Equation", EquationError_t::set_value_requested, "Trying to affect a value to a dynamically determined equation");
}

EquationComputationFailureCause_t Equation::getComputationFailureCause() const
{
	return this->failureCause;
}

bool Equation::isInverted() const
{
	bool result = false;

	switch (function)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
		result = true;
		break;
	case OperatorType_t::identity:
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::concatOp:
	case OperatorType_t::constant:
		break;
	}

	return result;
}

int Equation::getRangeL() const
{
	return this->rangeL;
}

int Equation::getRangeR() const
{
	return this->rangeR;
}

void Equation::setInitialValue(const LogicValue&)
{
	throw StatesException("Equation", EquationError_t::set_value_requested, "Trying to affect an initial value to an equation");
}

void Equation::setCurrentValue(const LogicValue&)
{
	throw StatesException("Equation", EquationError_t::set_value_requested, "Trying to affect a current value to an equation");
}

void Equation::setCurrentValueSubRange(const LogicValue&, int, int)
{
	throw StatesException("Equation", EquationError_t::set_value_requested, "Trying to affect a current value to an equation");
}

/**
 * @brief Equation::getOperands
 * Warning: this function returns pointers to Equation operands.
 * This must not be used to take ownership of operands.
 * @return
 */
QVector<shared_ptr<Variable>> Equation::getOperands() const
{
	QVector<shared_ptr<Variable>> operands(this->allowedOperandCount);

	for (uint i = 0 ; i < this->allowedOperandCount ; i++)
	{
		if (!signalOperands[i].expired())
			operands[i] = signalOperands[i].lock();
		else if (equationOperands[i] != nullptr)
			operands[i] = equationOperands[i];
	}

	return operands;
}

void Equation::computeCurrentValue()
{
	// Current value is computed dynamically when operands or
	// equation change.
	// It emits change events only if value actually changed

	bool doCompute = true;

	uint operandsSize = 0;

	for (shared_ptr<Variable> currentOperand : this->getOperands())
	{
		if (currentOperand == nullptr)
		{
			this->failureCause = EquationComputationFailureCause_t::nullOperand;
			doCompute = false;
			break;
		}
		else if ( currentOperand->getSize() == 0)
		{
			this->failureCause = EquationComputationFailureCause_t::incompleteOperand;
			doCompute = false;
			break;
		}
		else if ( (operandsSize != 0) && (currentOperand->getSize() != operandsSize) )
		{
			if (this->function != OperatorType_t::concatOp)
			{
				this->failureCause = EquationComputationFailureCause_t::sizeMismatch;
				doCompute = false;
				break;
			}
		}
		else if (operandsSize == 0)
			operandsSize = currentOperand->getSize();
	}

	LogicValue previousValue = this->currentValue;

	if (doCompute)
	{
		QVector<shared_ptr<Variable>> operands = this->getOperands();
		this->failureCause = EquationComputationFailureCause_t::nofail;

		switch (this->function)
		{
		case OperatorType_t::notOp:
			this->currentValue = ! ( operands[0]->getCurrentValue() );
			break;
		case OperatorType_t::identity:
			this->currentValue = operands[0]->getCurrentValue();
			break;
		case OperatorType_t::equalOp:
		case OperatorType_t::diffOp:
		{
			LogicValue oneBitResult(1);
			if (function == OperatorType_t::equalOp)
			{
				oneBitResult[0] = ((operands[0]->getCurrentValue() == operands[1]->getCurrentValue()));
			}
			else if (function == OperatorType_t::diffOp)
			{
				oneBitResult[0] = ((operands[0]->getCurrentValue() != operands[1]->getCurrentValue()));
			}

			this->currentValue = oneBitResult;
		}
			break;
		case OperatorType_t::extractOp:
			if (this->rangeL != -1)
			{
				if (this->rangeR != -1)
				{
					if (this->rangeL >= this->rangeR)
					{
						int range = this->rangeL - this->rangeR + 1;
						LogicValue subVector(range);
						LogicValue originalValue = operands[0]->getCurrentValue();

						for (int i = 0 ; i < range ; i++)
						{
							subVector[i] = originalValue[this->rangeR + i];
						}

						this->currentValue = subVector;

					}
					else
					{
						this->currentValue = LogicValue::getNullValue();
						this->failureCause = EquationComputationFailureCause_t::incorrectParameter;
					}
				}
				else
				{
					if ((uint)rangeL < operands[0]->getSize())
					{
						LogicValue result(1);
						result[0] = operands[0]->getCurrentValue()[rangeL];
						this->currentValue = result;
					}
					else
					{
						rangeL = -1;
						this->currentValue = LogicValue::getNullValue();
						this->failureCause = EquationComputationFailureCause_t::incorrectParameter;
					}
				}
			}
			else
			{
				this->currentValue = LogicValue::getNullValue();
				this->failureCause = EquationComputationFailureCause_t::missingParameter;
			}
			break;
		case OperatorType_t::concatOp:
		{
			int sizeCount = 0;
			for (shared_ptr<Variable> currentOperand : this->getOperands())
			{
				sizeCount += currentOperand->getSize();
			}

			LogicValue concatVector(sizeCount);

			int currentBit = sizeCount - 1;
			for (shared_ptr<Variable> currentOperand : this->getOperands())
			{
				for (int i = currentOperand->getSize()-1 ; i >= 0 ; i--)
				{
					concatVector[currentBit] = currentOperand->getCurrentValue()[i];
					currentBit--;
				}
			}

			this->currentValue = concatVector;
		}
			break;
		case OperatorType_t::andOp:
		case OperatorType_t::nandOp:
		{
			LogicValue partialResult(operandsSize, true);
			for (shared_ptr<Variable> operand : operands)
			{
				partialResult &= operand->getCurrentValue();
			}

			if (this->isInverted())
				partialResult = !partialResult;

			this->currentValue = partialResult;
		}
			break;

		case OperatorType_t::orOp:
		case OperatorType_t::norOp:
		{
			LogicValue partialResult(operandsSize);
			for (shared_ptr<Variable> operand : operands)
			{
				partialResult |= operand->getCurrentValue();
			}

			if (this->isInverted())
				partialResult = !partialResult;

			this->currentValue = partialResult;
		}
			break;

		case OperatorType_t::xorOp:
		case OperatorType_t::xnorOp:
		{
			LogicValue partialResult(operandsSize);
			for (shared_ptr<Variable> operand : operands)
			{
				partialResult ^= operand->getCurrentValue();
			}

			if (this->isInverted())
				partialResult = !partialResult;

			this->currentValue = partialResult;
		}
			break;
		case OperatorType_t::constant:
			this->currentValue = this->constantValue;
			break;
		}
	}
	else
	{
		this->currentValue = LogicValue::getNullValue();
	}

	if (previousValue != this->currentValue)
		emit variableDynamicStateChangedEvent();

	if (previousValue.getSize() != this->currentValue.getSize())
	{
		try
		{
			emit variableResizedEvent();
		}
		catch (const std::bad_weak_ptr&)
		{
			// It's ok to fail here: it means shared_from_this is illegal to call.
			// This happens when this function is called from the constructor
		}
	}
}

void Equation::signalDeletedEventHandler()
{
	this->computeCurrentValue();
	emit this->variableStaticConfigurationChangedEvent();
}

bool Equation::signalHasSize(shared_ptr<Variable> sig)
{
	if (sig == nullptr)
		return false;
	else if (sig->getSize() == 0)
		return false;
	else
		return true;
}

/**
 * @brief Equation::decreaseOperandCountInternal Decreases the operand count without
 * checking on the function required operands. It still makes sure the operand count
 * doesn't go below 0.
 * This operation can place the object in a state where function does not have it
 * required count of operand, so the caller has to make sure he replaces the object
 * in a correct state before any call to value computing is made.
 */
void Equation::decreaseOperandCountInternal() // Throws StatesException
{
	// Do not allow placing an operand outside defined range
	if (this->allowedOperandCount == 0)
	{
		throw StatesException("Equation", EquationError_t::reduced_operand_while_0, "Trying to reduce operand count while count is 0");
	}

	clearOperand(this->allowedOperandCount-1); // Throws StatesException - Operand count checked - ignored
	this->allowedOperandCount--;

	this->signalOperands.resize(this->allowedOperandCount);
	this->equationOperands.resize(this->allowedOperandCount);
}

/**
 * @brief Equation::increaseOperandCountInternal Increases the operand count without
 * checking on the function required operands.
 * This operation can place the object in a state where function does not have it
 * required count of operand, so the caller has to make sure he replaces the object
 * in a correct state before any call to value computing is made.
 */
void Equation::increaseOperandCountInternal()
{
	this->allowedOperandCount++;

	this->signalOperands.resize(this->allowedOperandCount);
	this->equationOperands.resize(this->allowedOperandCount);
}

uint Equation::getOperandCount() const
{
	return this->allowedOperandCount;
}

void Equation::increaseOperandCount() // Throws StatesException
{
	switch (function)
	{
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		this->increaseOperandCountInternal();

		emit variableStaticConfigurationChangedEvent();

		this->computeCurrentValue();
		break;
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::constant:
		throw StatesException("Equation", EquationError_t::change_operand_illegal, "Trying to change the operand count on a fixed operand count function");
		break;
	}
}

void Equation::decreaseOperandCount() // Throws StatesException
{
	switch (function)
	{
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		if (  (this->allowedOperandCount > 2) )
		{
			this->decreaseOperandCountInternal(); // Throws StatesException - Operand count cheked - ignored

			emit variableStaticConfigurationChangedEvent();

			this->computeCurrentValue();
		}

		break;
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::constant:
		throw StatesException("Equation", EquationError_t::change_operand_illegal, "Trying to change the operand count on a fixed operand count function");
		break;
	}
}
