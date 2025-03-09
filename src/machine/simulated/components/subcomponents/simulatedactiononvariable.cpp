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
#include "simulatedactiononvariable.h"

// StateS classes
#include "machinemanager.h"
#include "simulatedmachine.h"
#include "actiononvariable.h"
#include "simulatedvariable.h"


SimulatedActionOnVariable::SimulatedActionOnVariable(shared_ptr<const ActionOnVariable> sourceAction)
{
	this->variableId  = sourceAction->getVariableActedOnId();
	this->actionType  = sourceAction->getActionType();
	this->actionValue = sourceAction->getActionValue();
	this->rangeL      = sourceAction->getActionRangeL();
	this->rangeR      = sourceAction->getActionRangeR();
}

LogicValue SimulatedActionOnVariable::getActionValue() const
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return LogicValue::getNullValue();

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return LogicValue::getNullValue();


	LogicValue publicActionValue = LogicValue::getNullValue();
	switch (this->actionType)
	{
	case ActionOnVariableType_t::reset:
		publicActionValue = LogicValue::getValue0(this->getActionSize());
		break;
	case ActionOnVariableType_t::set:
		publicActionValue = LogicValue::getValue1(this->getActionSize());
		break;
	case ActionOnVariableType_t::activeOnState:
	case ActionOnVariableType_t::pulse:
		if (this->getActionSize() == 1)
		{
			publicActionValue = LogicValue::getValue1(this->getActionSize());
		}
		else
		{
			publicActionValue = this->actionValue;
		}
		break;
	case ActionOnVariableType_t::increment:
		publicActionValue = simulatedVariable->getCurrentValue();
		publicActionValue.increment();
		break;
	case ActionOnVariableType_t::decrement:
		publicActionValue = simulatedVariable->getCurrentValue();
		publicActionValue.decrement();
		break;
	case ActionOnVariableType_t::assign:
		publicActionValue = this->actionValue;
		break;
	}

	return publicActionValue;
}

void SimulatedActionOnVariable::beginAction()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return;


	simulatedVariable->setCurrentValueSubRange(this->getActionValue(), this->rangeL, this->rangeR);

	switch (this->actionType)
	{
	case ActionOnVariableType_t::pulse:
	case ActionOnVariableType_t::activeOnState:
		this->isActionActing = true;
		break;
	case ActionOnVariableType_t::reset:
	case ActionOnVariableType_t::set:
	case ActionOnVariableType_t::assign:
	case ActionOnVariableType_t::increment:
	case ActionOnVariableType_t::decrement:
		// Do not register, value change is definitive
		break;
	}
}

void SimulatedActionOnVariable::endAction()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return;


	if (this->isActionActing == true)
	{
		simulatedVariable->setCurrentValueSubRange(LogicValue::getValue0(this->getActionSize()), this->rangeL, this->rangeR);
		this->isActionActing = false;
	}
}

void SimulatedActionOnVariable::resetAction()
{
	this->isActionActing = false;
}

bool SimulatedActionOnVariable::isActionMemorized() const
{
	switch (this->actionType)
	{
	case ActionOnVariableType_t::set:
	case ActionOnVariableType_t::reset:
	case ActionOnVariableType_t::assign:
	case ActionOnVariableType_t::increment:
	case ActionOnVariableType_t::decrement:
		return true;
		break;
	case ActionOnVariableType_t::activeOnState:
	case ActionOnVariableType_t::pulse:
		return false;
		break;
	}
}

uint SimulatedActionOnVariable::getActionSize() const
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return 0;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return 0;


	if (simulatedVariable->getSize() == 1)
	{
		return 1;
	}
	else
	{
		if ( (this->rangeL < 0) && (this->rangeR < 0) )
		{
			return simulatedVariable->getSize();
		}
		else if ( (this->rangeL >= 0) && (this->rangeR < 0) )
		{
			return 1;
		}
		else
		{
			return (uint)(this->rangeL - this->rangeR + 1);
		}
	}
}
