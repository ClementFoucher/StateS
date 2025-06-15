/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIMULATEDEQUATION_H
#define SIMULATEDEQUATION_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// States classes
#include "statestypes.h"
#include "logicvalue.h"
class Equation;
class SimulatedVariable;
class SimulatedOperand;


/**
 * @brief
 * An equation is a gathering of operands linked by an operator.
 * Equations are thus "dynamic variables", which value will
 * depend on the values of its operands.
 *
 * Equation size in bits is also dynamic and depends on operands size,
 * except for equality and difference operators, which size is always 1.
 *
 * An equation with any of its operands undefined always returns
 * an undefined value.
 */
class SimulatedEquation : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedEquation(shared_ptr<const Equation> sourceEquation);

	/////
	// Object functions
public:
	LogicValue getCurrentValue() const;

	// Concept of true is only applicable to size 1 results
	// An equation whose result size is > 1 will never be true
	bool isTrue() const;

private slots:
	void computeCurrentValue();

private:
	bool isInverted() const;

	shared_ptr<SimulatedOperand> getOperand(uint i) const;
	uint getOperandCount() const;

	/////
	// Signals
signals:
	void equationCurrentValueChangedEvent();

	/////
	// Object variables
private:
	// Equation parameters
	OperatorType_t operatorType;
	QList<shared_ptr<SimulatedOperand>> operands;
	bool isValid = true;

	// Parameters specific to Extract operator type
	int rangeL = -1;
	int rangeR = -1;

	// Equation state recomputed dynamically
	LogicValue currentValue;

};

#endif // SIMULATEDEQUATION_H
