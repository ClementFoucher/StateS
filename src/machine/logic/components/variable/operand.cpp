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

// Current class header
#include "operand.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "equation.h"


Operand::Operand(componentId_t variableId) :
	Operand(OperandSource_t::variable)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	this->variableId = variableId;

	connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Operand::operandInitialValueChangedEvent);
	connect(variable.get(), &Variable::variableCurrentValueChangedEvent, this, &Operand::operandCurrentValueChangedEvent);
	connect(variable.get(), &Variable::variableRenamedEvent,             this, &Operand::operandTextChangedEvent);

	connect(variable.get(), &Variable::componentDeletedEvent, this, &Operand::variableDeletedEventHandler);
}

Operand::Operand(shared_ptr<Equation> equation) :
	Operand(OperandSource_t::equation)
{
	this->equation = equation;

	connect(equation.get(), &Equation::equationInitialValueChangedEvent, this, &Operand::operandInitialValueChangedEvent);
	connect(equation.get(), &Equation::equationCurrentValueChangedEvent, this, &Operand::operandCurrentValueChangedEvent);
	connect(equation.get(), &Equation::equationTextChangedEvent,         this, &Operand::operandTextChangedEvent);
}

Operand::Operand(LogicValue constant) :
	Operand(OperandSource_t::constant)
{
	this->constant = constant;
}

Operand::Operand(shared_ptr<Variable> variable) :
	Operand(OperandSource_t::variable)
{
	if (variable == nullptr) return;


	this->variableId = variable->getId();

	connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Operand::operandInitialValueChangedEvent);
	connect(variable.get(), &Variable::variableCurrentValueChangedEvent, this, &Operand::operandCurrentValueChangedEvent);
	connect(variable.get(), &Variable::variableRenamedEvent,             this, &Operand::operandTextChangedEvent);

	connect(variable.get(), &Variable::componentDeletedEvent, this, &Operand::variableDeletedEventHandler);
}

Operand::Operand(OperandSource_t operandSource)
{
	// The source MUST be defined, even if there is a failure on the machine
	// This private constructor ensures it.

	this->source = operandSource;
}

shared_ptr<Operand> Operand::clone() const
{
	switch (this->source)
	{
	case OperandSource_t::variable:
		return shared_ptr<Operand>(new Operand(this->variableId));
		break;
	case OperandSource_t::equation:
		return shared_ptr<Operand>(new Operand(this->equation->clone()));
		break;
	case OperandSource_t::constant:
		return shared_ptr<Operand>(new Operand(this->constant));
		break;
	}
}

OperandSource_t Operand::getSource() const
{
	return this->source;
}

LogicValue Operand::getInitialValue() const
{
	switch (this->source)
	{
	case OperandSource_t::variable:
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return LogicValue::getNullValue();

		auto variable = machine->getVariable(this->variableId);
		if (variable == nullptr) return LogicValue::getNullValue();


		return variable->getInitialValue();
		break;
	}
	case OperandSource_t::equation:
		if (this->equation == nullptr) return LogicValue::getNullValue();


		return this->equation->getInitialValue();
		break;
	case OperandSource_t::constant:
		return this->constant;
		break;
	}
}

LogicValue Operand::getCurrentValue() const
{
	switch (this->source)
	{
	case OperandSource_t::variable:
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return LogicValue::getNullValue();

		auto variable = machine->getVariable(this->variableId);
		if (variable == nullptr) return LogicValue::getNullValue();


		return variable->getCurrentValue();
		break;
	}
	case OperandSource_t::equation:
		if (this->equation == nullptr) return LogicValue::getNullValue();


		return this->equation->getCurrentValue();
		break;
	case OperandSource_t::constant:
		return this->constant;
		break;
	}
}

componentId_t Operand::getVariableId() const
{
	if (this->source != OperandSource_t::variable) return nullId;


	return this->variableId;
}

shared_ptr<Equation> Operand::getEquation() const
{
	if (this->source != OperandSource_t::equation) return nullptr;


	return this->equation;
}

LogicValue Operand::getConstant() const
{
	if (this->source != OperandSource_t::constant) return LogicValue::getNullValue();


	return this->constant;
}

QString Operand::getText() const
{
	switch (this->source)
	{
	case OperandSource_t::variable:
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return QString();

		auto variable = machine->getVariable(this->variableId);
		if (variable == nullptr) return QString();


		return variable->getName();
		break;
	}
	case OperandSource_t::equation:
		if (this->equation == nullptr) return QString();


		return this->equation->getText();
		break;
	case OperandSource_t::constant:
		if (this->constant.isNull()) return "...";


		return this->constant.toString();
		break;
	}
}

void Operand::variableDeletedEventHandler(componentId_t)
{
	this->variableId = nullId;
	emit this->operandInvalidatedEvent();
}
