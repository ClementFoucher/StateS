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

void SimulatedActionOnVariable::doAction()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return;


	simulatedVariable->setCurrentValueSubRange(this->getActionValue(), this->rangeL, this->rangeR);
}

bool SimulatedActionOnVariable::isActionMemorized() const
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return false;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return false;


	return simulatedVariable->getMemorized();
}

componentId_t SimulatedActionOnVariable::getVariableId() const
{
	return this->variableId;
}

LogicValue SimulatedActionOnVariable::getActionValue() const
{
	switch (this->actionType)
	{
	case ActionOnVariableType_t::reset:
	case ActionOnVariableType_t::set:
	case ActionOnVariableType_t::continuous:
	case ActionOnVariableType_t::pulse:
	case ActionOnVariableType_t::assign:
		return this->actionValue;
		break;
	case ActionOnVariableType_t::increment:
	case ActionOnVariableType_t::decrement:
	{
		auto simulatedMachine = machineManager->getSimulatedMachine();
		if (simulatedMachine == nullptr) return LogicValue::getNullValue();

		auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
		if (simulatedVariable == nullptr) return LogicValue::getNullValue();


		auto publicActionValue = simulatedVariable->getCurrentValue();
		if (this->actionType == ActionOnVariableType_t::increment)
		{
			publicActionValue.increment();
		}
		else // (this->actionType == ActionOnVariableType_t::decrement)
		{
			publicActionValue.decrement();
		}
		return publicActionValue;
		break;
	}
	case ActionOnVariableType_t::none:
		return LogicValue::getNullValue();
		break;
	}
}
