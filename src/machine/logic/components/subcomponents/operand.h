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

#ifndef OPERAND_H
#define OPERAND_H

// Parent
#include <QObject>

// Stdlib
#include <memory>
#include <variant>
using namespace std;

// StateS
#include "componentid.h"
#include "machinevalue.h"
class Equation;
class Variable;


class Operand : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class Source_t
	{
		variable, // Reference (using componentId) to a variable defined in the machine (including constants).
		equation, // Operand is itself an equation.
		constant  // Constant defined on-the-fly in equation editor. Not to confuse with constants defined in the machine, which are variables (try to follow here!).
	};

	/////
	// Constructors/destructors
public:
	explicit Operand(ComponentId variableId);        // Defines an operand whose source is a variable
	explicit Operand(shared_ptr<Equation> equation); // Defines an operand whose source is an equation
	explicit Operand(MachineValue constant);         // Defines an operand whose source is a constant
	explicit Operand(shared_ptr<Variable> variable); // Defines an operand whose source is a variable (when machine is still being parsed)

	/////
	// Object functions
public:
	shared_ptr<Operand> clone() const;

	Source_t getSource() const;

	MachineValue getInitialValue() const;
	MachineValue::Type_t getType() const;

	ComponentId          getVariableId() const;
	shared_ptr<Equation> getEquation()   const;
	MachineValue         getConstant()   const;

	QString getText() const;

private slots:
	void variableDeletedEventHandler(ComponentId);

	/////
	// Signals
signals:
	void operandInitialValueChangedEvent();
	void operandTextChangedEvent();
	void operandInvalidatedEvent();

	/////
	// Object variables
private:
	Source_t source;

	std::variant<ComponentId, shared_ptr<Equation>, MachineValue> value;

};

#endif // OPERAND_H
