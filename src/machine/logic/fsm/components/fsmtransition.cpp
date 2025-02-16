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

void FsmTransition::setCondition(shared_ptr<Variable> variableNewCondition)
{
	if (this->condition != nullptr)
	{
		disconnect(this->condition.get(), &Variable::variableStaticConfigurationChangedEvent, this, &FsmTransition::conditionChangedEventHandler);
	}

	shared_ptr<Equation> equationNewCondition = dynamic_pointer_cast<Equation>(variableNewCondition);
	if (equationNewCondition == nullptr)
	{
		QVector<shared_ptr<Variable>> operand;
		operand.append(variableNewCondition);
		equationNewCondition = shared_ptr<Equation>(new Equation(OperatorType_t::identity, operand));
	}

	this->condition = equationNewCondition;

	if (this->condition != nullptr)
	{
		// Propagate events
		connect(this->condition.get(), &Variable::variableStaticConfigurationChangedEvent, this, &FsmTransition::conditionChangedEventHandler);
	}

	emit this->conditionChangedEvent();
	emit this->componentEditedEvent(this->id);
}

void FsmTransition::clearCondition()
{
	this->setCondition(nullptr);
}

shared_ptr<Variable> FsmTransition::getCondition() const
{
	if (this->condition != nullptr)
	{
		if (this->condition->getFunction() != OperatorType_t::identity)
		{
			return this->condition;
		}
		else
		{
			return this->condition->getOperand(0); // Throws StatesException - Identity op always has operand 0, even if nullptr - ignored
		}
	}
	else
	{
		return nullptr;
	}
}

uint FsmTransition::getAllowedActionTypes() const
{
	return ((uint)actuatorAllowedActionType_t::pulse     |
	        (uint)actuatorAllowedActionType_t::set       |
	        (uint)actuatorAllowedActionType_t::reset     |
	        (uint)actuatorAllowedActionType_t::assign    |
	        (uint)actuatorAllowedActionType_t::increment |
	        (uint)actuatorAllowedActionType_t::decrement
	       );
}

void FsmTransition::conditionChangedEventHandler()
{
	emit this->conditionChangedEvent();
	emit this->componentEditedEvent(this->id);
}
