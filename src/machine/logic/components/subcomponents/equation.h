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

#ifndef EQUATION_H
#define EQUATION_H

// Parent
#include <QObject>

// Stdlib
#include <memory>
using namespace std;

// StateS
#include "componentid.h"
#include "machinevalue.h"
class Variable;
class Operand;


/**
 * @brief
 * An equation is a gathering of operands linked by an operator.
 * Equations are thus "dynamic variables", whose value will
 * depend on the values of its operands.
 *
 * An equation that have any of its operands undefined or erroneous
 * always returns a null value.
 */
class Equation : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class Operator_t
	{
		notOp,  // Not equations always have exactly one operand
		andOp,
		orOp,
		xorOp,
		nandOp,
		norOp,
		xnorOp,
		equalOp, // Equal equations always have exactly two operand and are size one
		diffOp,  // Diff  equations always have exactly two operand and are size one
		extractOp, // Extract equations always have exacly one operand
		concatOp,
		identity // For internal use only, exactly one operand
	};

	enum class ComputationFailureCause_t
	{
		nofail,
		nullOperand,
		invalidOperandValue,
		incorrectOperandType,
		operandsSizesMismatch,
		operandsTypesMismatch,
		missingParameter,
		incorrectParameterValue
	};

	enum class ComputationWarning_t
	{
		noWarning,
		differentTypeComparison,
		differentSizeComparison
	};

	/////
	// Constructors/destructors
public:
	explicit Equation(Operator_t operatorType, int operandCount = -1);

	/////
	// Object functions
public:
	shared_ptr<Equation> clone() const;

	bool isValid() const;
	MachineValue::Type_t getType() const;

	MachineValue getInitialValue() const;

	QString getText() const;
	QString getColoredText(bool raw = false) const;

	ComputationFailureCause_t getComputationFailureCause() const;
	ComputationWarning_t      getComputationWarning()      const;

	void setOperator(Operator_t newOperator);
	Operator_t getOperator() const;

	QSet<ComponentId> getVariablesIdsSet() const;

	bool isInverted() const;

	shared_ptr<Operand> getOperand(uint i) const;
	void setOperand(uint i, ComponentId          newOperand); // Set variable operand
	void setOperand(uint i, shared_ptr<Equation> newOperand); // Set equation operand
	void setOperand(uint i, MachineValue         newOperand); // Set constant operand
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
	void checkForErrors();
	MachineValue computeInitialValue();

	/////
	// Signals
signals:
	void equationInitialValueChangedEvent();
	void equationTextChangedEvent();
	void equationInvalidatedEvent();

	/////
	// Object variables
private:
	// Equation parameters
	Operator_t operatorType;
	QList<shared_ptr<Operand>> operands;

	// Parameters specific to Extract operator type
	int rangeL = -1;
	int rangeR = -1;

	// Equation state recomputed dynamically
	MachineValue initialValue{};

	ComputationFailureCause_t failureCause;
	ComputationWarning_t      warning;

};

#endif // EQUATION_H
