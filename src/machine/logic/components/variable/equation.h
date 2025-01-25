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
#include "variable.h"

// C++ classes
#include <memory>
using namespace std;

// States classes
#include "statestypes.h"


/**
 * @brief
 * An equation is a gathering of variables and other equations, linked by an operator.
 * Equations are thus "compouned variables", which value is dynamic and depends
 * on the values of operands at each moment.
 *
 * Equation size in bits is also dynamic and depends on operands size,
 * except for equality and difference operators, which size is always 1.
 *
 * An equation can store as operand:
 * - nullptr => operand is not set
 * - machine variables
 * - equations with a size
 * - equations with no size
 *
 * An equation with any of its operands set to nullptr or to unsized equations
 * will have no visible size, and always returns a null logic value.
 * An equation with operands of different sizes will behave the same.
 *
 * An equation store its own copy (cloned at initialization) of Equation operands => shared_ptr.
 * Machine variables are primary held by the machine => weak_ptr.
 *
 */
class Equation : public Variable
{
	Q_OBJECT

	/////
	// Static functions
private:
	static bool variableHasSize(shared_ptr<Variable> sig);

	/////
	// Constructors/destructors
public:
	explicit Equation(OperatorType_t function, int allowedOperandCount = -1);
	explicit Equation(OperatorType_t function, const QVector<shared_ptr<Variable>>& operandList);

	/////
	// Object functions
public:
	shared_ptr<Equation> clone() const;

	virtual uint getSize() const override;
	virtual void resize(uint)    override; // Throws StatesException

	virtual QString getText() const override;
	QString getColoredText(bool raw = false) const;

	EquationComputationFailureCause_t getComputationFailureCause() const;

	OperatorType_t getFunction() const;
	void setFunction(OperatorType_t newFunction);

	bool isInverted() const;

	shared_ptr<Variable> getOperand(uint i) const; // Throws StatesException
	bool setOperand(uint i, shared_ptr<Variable> newOperand, bool quiet = false); // Throws StatesException
	void clearOperand(uint i, bool quiet = false); // Throws StatesException

	QVector<shared_ptr<Variable>> getOperands() const;

	uint getOperandCount() const;
	void increaseOperandCount(); // Throws StatesException
	void decreaseOperandCount(); // Throws StatesException

	// Functions specific to some action types
	void setConstantValue(const LogicValue& value); // Throws StatesException
	void setRange(int rangeL, int rangeR = -1); // TODO: throw exception when function is not extract? Or simply qDebug... What about out of range values?
	int getRangeL() const; // TODO: throw exception when function is not extract?
	int getRangeR() const; // TODO: throw exception when function is not extract?

	// Override to throw exception
	virtual void setInitialValue(const LogicValue&)                   override; // Throws StatesException
	virtual void setCurrentValue(const LogicValue&)                   override; // Throws StatesException
	virtual void setCurrentValueSubRange(const LogicValue&, int, int) override; // Throws StatesException

private slots:
	void computeCurrentValue();
	void variableDeletedEventHandler();

private:
	void increaseOperandCountInternal();
	void decreaseOperandCountInternal(); // Throws StatesException

	/////
	// Object variables
private:
	EquationComputationFailureCause_t failureCause = EquationComputationFailureCause_t::uncomputed;

	OperatorType_t function;
	// Different storage for different ownership (weak/shared)
	QVector<weak_ptr<Variable>>   variableOperands;
	QVector<shared_ptr<Equation>> equationOperands;

	// This size holds the maximum operands count
	// It can be increased or decreased (min 2 operands)
	// except for constant size operators (ident, not, eq, diff, constant)
	uint allowedOperandCount = 0;

	// Parameters specific to some action types
	int rangeL;
	int rangeR;
	LogicValue constantValue;

};

#endif // EQUATION_H
