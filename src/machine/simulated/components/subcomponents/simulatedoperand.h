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

#ifndef SIMULATEDOPERAND_H
#define SIMULATEDOPERAND_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class Operand;
class SimulatedEquation;


class SimulatedOperand : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedOperand(shared_ptr<const Operand> sourceOperand);

	/////
	// Object functions
public:
	LogicValue getCurrentValue() const;

	/////
	// Signals
signals:
	void operandCurrentValueChangedEvent();

	/////
	// Object variables
private:
	OperandSource_t source;

	componentId_t                 variableId = nullId;
	shared_ptr<SimulatedEquation> equation;
	LogicValue                    constant   = LogicValue();

};

#endif // SIMULATEDOPERAND_H
