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

#ifndef EQUATION_H
#define EQUATION_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// States classes
#include "statestypes.h"
#include "logicvalue.h"
class Variable;
class Operand;


/**
 * @brief
 * An equation is a gathering of operands linked by an operator.
 * Equations are thus "dynamic variables", which value will
 * depend on the values of its operands.
 *
 * Equation size in bits is also dynamic and depends on operands size,
 * except for equality and difference operators, which size is always 1.
 *
 * An equation with any of its operands undefined or erroneous
 * always returns a null value.
 */
class Equation : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Equation(OperatorType_t operatorType, int operandCount = -1);

	/////
	// Object functions
public:
	shared_ptr<Equation> clone() const;

	uint getSize() const;

	LogicValue getInitialValue() const;

	QString getText() const;
	QString getColoredText(bool raw = false) const;

	EquationComputationFailureCause_t getComputationFailureCause() const;

	void setOperatorType(OperatorType_t newOperator);
	OperatorType_t getOperatorType() const;

	bool isInverted() const;

	shared_ptr<Operand> getOperand(uint i) const;
	void setOperand(uint i, componentId_t newOperand);        // Set variable operand
	void setOperand(uint i, shared_ptr<Equation> newOperand); // Set equation operand
	void setOperand(uint i, LogicValue newOperand);           // Set constant operand
	void setOperand(uint i, shared_ptr<Variable> newOperand); // Set variable operand (when machine is still being parsed)
	void clearOperand(uint i);

	uint getOperandCount() const;
	void increaseOperandCount();
	void decreaseOperandCount();

	// Functions specific to Extract operator type
	void setRange(int rangeL, int rangeR = -1);
	int getRangeL() const;
	int getRangeR() const;

	void doFullStackRecomputation();

private slots:
	void checkAndComputeInitialValue();
	void operandInvalidatedEventHandler();

private:
	void setOperand(uint i, shared_ptr<Operand> newOperand);
	LogicValue computeInitialValue();

	/////
	// Signals
signals:
	void equationInitialValueChangedEvent();
	void equationCurrentValueChangedEvent();
	void equationTextChangedEvent();
	void equationInvalidatedEvent();

	/////
	// Object variables
private:
	// Equation parameters
	OperatorType_t operatorType;
	QList<shared_ptr<Operand>> operands;

	// Parameters specific to Extract operator type
	int rangeL = -1;
	int rangeR = -1;

	// Equation state recomputed dynamically
	LogicValue initialValue;
	EquationComputationFailureCause_t failureCause;

};

#endif // EQUATION_H
