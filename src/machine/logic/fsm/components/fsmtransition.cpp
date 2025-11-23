/*
 * Copyright © 2014-2025 Clément Foucher
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
 * along with StateS. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "fsmtransition.h"

// StateS classes
#include "equation.h"


FsmTransition::FsmTransition(componentId_t sourceStateId, componentId_t targetStateId) :
	MachineActuatorComponent()
{
	this->sourceStateId = sourceStateId;
	this->targetStateId = targetStateId;
}

FsmTransition::FsmTransition(componentId_t id, componentId_t sourceStateId, componentId_t targetStateId) :
	MachineActuatorComponent(id)
{
	this->sourceStateId = sourceStateId;
	this->targetStateId = targetStateId;
}

void FsmTransition::setSourceStateId(componentId_t sourceStateId)
{
	this->sourceStateId = sourceStateId;
	// Do not emit componentEditedEvent signal as the graphic
	// transition is recreated by the caller (FsmScene)
}

componentId_t FsmTransition::getSourceStateId() const
{
	return this->sourceStateId;
}

void FsmTransition::setTargetStateId(componentId_t targetStateId)
{
	this->targetStateId = targetStateId;
	// Do not emit componentEditedEvent signal as the graphic
	// transition is recreated by the caller (FsmScene)
}

componentId_t FsmTransition::getTargetStateId() const
{
	return this->targetStateId;
}

void FsmTransition::setCondition(shared_ptr<Equation> newCondition)
{
	if (this->condition != nullptr)
	{
		disconnect(this->condition.get(), &Equation::equationTextChangedEvent,         this, &FsmTransition::conditionChangedEventHandler);
		disconnect(this->condition.get(), &Equation::equationInitialValueChangedEvent, this, &FsmTransition::conditionChangedEventHandler);
		disconnect(this->condition.get(), &Equation::equationInvalidatedEvent,         this, &FsmTransition::conditionInvalidatedEventHandler);
	}

	this->condition = newCondition;

	if (this->condition != nullptr)
	{
		connect(this->condition.get(), &Equation::equationTextChangedEvent,         this, &FsmTransition::conditionChangedEventHandler);
		// Initial value change needs to be connected in case a signal changes size
		connect(this->condition.get(), &Equation::equationInitialValueChangedEvent, this, &FsmTransition::conditionChangedEventHandler);
		connect(this->condition.get(), &Equation::equationInvalidatedEvent,         this, &FsmTransition::conditionInvalidatedEventHandler);
	}

	emit this->conditionChangedEvent();
	emit this->componentEditedEvent(this->id);
}

void FsmTransition::clearCondition()
{
	this->setCondition(nullptr);
}

shared_ptr<Equation> FsmTransition::getCondition() const
{
	return this->condition;
}

uint FsmTransition::getAllowedActionTypes() const
{
	return ((uint)ActionOnVariableType_t::pulse     |
	        (uint)ActionOnVariableType_t::set       |
	        (uint)ActionOnVariableType_t::reset     |
	        (uint)ActionOnVariableType_t::assign    |
	        (uint)ActionOnVariableType_t::increment |
	        (uint)ActionOnVariableType_t::decrement
	       );
}

void FsmTransition::conditionChangedEventHandler()
{
	emit this->conditionChangedEvent();
	emit this->componentEditedEvent(this->id);
}

void FsmTransition::conditionInvalidatedEventHandler()
{
	this->clearCondition();
}
