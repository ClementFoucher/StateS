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

#ifndef OPERAND_H
#define OPERAND_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class Equation;
class Variable;


class Operand : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Operand(shared_ptr<Variable> variable); // Defines an operand whose source is a variable
	explicit Operand(shared_ptr<Equation> equation); // Defines an operand whose source is an equation
	explicit Operand(LogicValue constant);           // Defines an operand whose source is a constant

	/////
	// Object functions
public:
	shared_ptr<Operand> clone() const;

	OperandSource_t getSource() const;

	LogicValue getInitialValue() const;
	LogicValue getCurrentValue() const;

	shared_ptr<Variable> getVariable() const;
	shared_ptr<Equation> getEquation() const;
	LogicValue           getConstant() const;

	QString getText() const;

private slots:
	void variableAboutToBeDeletedEventHandler();

	/////
	// Signals
signals:
	void operandInitialValueChangedEvent();
	void operandCurrentValueChangedEvent();
	void operandTextChangedEvent();
	void operandAboutToBeInvalidatedEvent();

	/////
	// Object variables
private:
	OperandSource_t source;

	shared_ptr<Variable> variable;
	shared_ptr<Equation> equation;
	LogicValue           constant = LogicValue();

};

#endif // OPERAND_H
