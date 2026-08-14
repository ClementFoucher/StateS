/*
 * Copyright © 2025-2026 Clément Foucher
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

// Stdlib
#include <memory>

// StateS
#include "machinevalue.h"
#include "equation.h"
class SimulatedOperand;


class SimulatedEquation : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedEquation(std::shared_ptr<const Equation> sourceEquation);

	/////
	// Object functions
public:
	MachineValue getCurrentValue() const;

	// Concept of true is only applicable to size 1 results
	// An equation whose result size is > 1 will never be true
	bool isTrue() const;

private slots:
	void computeCurrentValue();

private:
	bool isInverted() const;

	/////
	// Signals
signals:
	void equationCurrentValueChangedEvent();

	/////
	// Object variables
private:
	// Equation parameters
	Equation::Operator_t operatorType;
	QList<std::shared_ptr<SimulatedOperand>> operands;

	// Parameters specific to Extract operator type
	int rangeL = -1;
	int rangeR = -1;

	// Equation state recomputed dynamically
	MachineValue currentValue{};

};

#endif // SIMULATEDEQUATION_H
