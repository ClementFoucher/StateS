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

#ifndef SIMULATEDOPERAND_H
#define SIMULATEDOPERAND_H

// Parent
#include <QObject>

// Stdlib
#include <memory>
#include <variant>
using namespace std;

// StateS
#include "operand.h"
#include "machinevalue.h"
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
	MachineValue getCurrentValue() const;

	MachineValue::Type_t getType() const;

	/////
	// Signals
signals:
	void operandCurrentValueChangedEvent();

	/////
	// Object variables
private:
	Operand::Source_t source;

	std::variant<componentId_t, shared_ptr<SimulatedEquation>, MachineValue> value;

};

#endif // SIMULATEDOPERAND_H
