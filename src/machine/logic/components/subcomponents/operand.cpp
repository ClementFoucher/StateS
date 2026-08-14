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

// Current class header
#include "operand.h"

// StateS
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "equation.h"


Operand::Operand(ComponentId variableId) :
	source{Source_t::variable},
	value{nullId}
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	this->value = variableId;

	connect(variable.get(), &Variable::variableTypeChangedEvent,         this, &Operand::operandInitialValueChangedEvent);
	connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Operand::operandInitialValueChangedEvent);
	connect(variable.get(), &Variable::variableRenamedEvent,             this, &Operand::operandTextChangedEvent);

	connect(variable.get(), &Variable::componentDeletedEvent, this, &Operand::variableDeletedEventHandler);
}

Operand::Operand(std::shared_ptr<Equation> equation) :
	source{Source_t::equation},
	value{equation}
{
	if (equation == nullptr) return;


	connect(equation.get(), &Equation::equationInitialValueChangedEvent, this, &Operand::operandInitialValueChangedEvent);
	connect(equation.get(), &Equation::equationTextChangedEvent,         this, &Operand::operandTextChangedEvent);
}

Operand::Operand(MachineValue constant) :
	source{Source_t::constant},
	value{constant}
{
}

Operand::Operand(std::shared_ptr<Variable> variable) :
	source{Source_t::variable},
	value{nullId}
{
	if (variable == nullptr) return;


	this->value = variable->getId();

	connect(variable.get(), &Variable::variableTypeChangedEvent,         this, &Operand::operandInitialValueChangedEvent);
	connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Operand::operandInitialValueChangedEvent);
	connect(variable.get(), &Variable::variableRenamedEvent,             this, &Operand::operandTextChangedEvent);

	connect(variable.get(), &Variable::componentDeletedEvent, this, &Operand::variableDeletedEventHandler);
}

std::shared_ptr<Operand> Operand::clone() const
{
	switch (this->source)
	{
	case Source_t::variable:
		return std::make_shared<Operand>(this->getVariableId());
		break;
	case Source_t::equation:
		return std::make_shared<Operand>(this->getEquation()->clone());
		break;
	case Source_t::constant:
		return std::make_shared<Operand>(this->getConstant());
		break;
	}
}

Operand::Source_t Operand::getSource() const
{
	return this->source;
}

MachineValue Operand::getInitialValue() const
{
	switch (this->source)
	{
	case Source_t::variable:
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return MachineValue{};

		auto variable = machine->getVariable(this->getVariableId());
		if (variable == nullptr) return MachineValue{};


		return variable->getInitialValue();
		break;
	}
	case Source_t::equation:
	{
		auto equation = this->getEquation();
		if (equation == nullptr) return MachineValue{};


		return equation->getInitialValue();
		break;
	}
	case Source_t::constant:
		return this->getConstant();
		break;
	}
}

MachineValue::Type_t Operand::getType() const
{
	return this->getInitialValue().getType();
}

ComponentId Operand::getVariableId() const
{
	if (this->source != Source_t::variable) return nullId;


	return std::get<ComponentId>(this->value);
}

std::shared_ptr<Equation> Operand::getEquation() const
{
	if (this->source != Source_t::equation) return nullptr;


	return std::get<std::shared_ptr<Equation>>(this->value);
}

MachineValue Operand::getConstant() const
{
	if (this->source != Source_t::constant) return MachineValue{};


	return std::get<MachineValue>(this->value);
}

QString Operand::getText() const
{
	switch (this->source)
	{
	case Source_t::variable:
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return QString();

		auto variable = machine->getVariable(this->getVariableId());
		if (variable == nullptr) return QString();


		return variable->getName();
		break;
	}
	case Source_t::equation:
	{
		auto equation = this->getEquation();
		if (equation == nullptr) return QString();


		return equation->getText();
		break;
	}
	case Source_t::constant:
	{
		auto constant = this->getConstant();
		if (constant.isNull() == true) return "...";


		return constant.toDisplayString();
		break;
	}
	}
}

void Operand::variableDeletedEventHandler(ComponentId)
{
	this->value = nullId;
	emit this->operandInvalidatedEvent();
}
