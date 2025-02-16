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
#include "machineactuatorcomponent.h"

// Qt classes
#include <QDebug>

// StateS classes
#include "statestypes.h"
#include "variable.h"
#include "actiononvariable.h"
#include "statesexception.h"
#include "exceptiontypes.h"


MachineActuatorComponent::MachineActuatorComponent() :
    MachineComponent()
{

}

MachineActuatorComponent::MachineActuatorComponent(componentId_t id) :
    MachineComponent(id)
{

}

shared_ptr<ActionOnVariable> MachineActuatorComponent::addAction(shared_ptr<Variable> variable)
{
	// Default action type
	ActionOnVariableType_t actionType;

	if ((this->getAllowedActionTypes() & (uint)actuatorAllowedActionType_t::continuous) != 0)
	{
		actionType = ActionOnVariableType_t::activeOnState;
	}
	else
	{
		actionType = ActionOnVariableType_t::pulse;
	}

	shared_ptr<ActionOnVariable> action(new ActionOnVariable(variable, actionType));
	connect(action.get(), &ActionOnVariable::actionChangedEvent, this, &MachineActuatorComponent::variableInActionListModifiedEventHandler);
	this->actionList.append(action);

	// To remove destroyed variables from the action list
	connect(variable.get(), &Variable::variableDeletedEvent, this, &MachineActuatorComponent::cleanActionList);

	emit this->actionListChangedEvent();
	emit this->componentEditedEvent(this->id);

	return action;
}

void MachineActuatorComponent::removeAction(uint actionRank) // Throws StatesException
{
	if (actionRank < (uint)this->actionList.count())
	{
		shared_ptr<Variable> variable = actionList.at(actionRank)->getVariableActedOn();
		if (variable != nullptr)
		{
			disconnect(variable.get(), &Variable::variableDeletedEvent, this, &MachineActuatorComponent::cleanActionList);
		}

		this->actionList.removeAt(actionRank);
		emit this->actionListChangedEvent();
		emit this->componentEditedEvent(this->id);
	}
	else
	{
		throw StatesException("MachineActuatorComponent", MachineActuatorComponentError_t::out_of_range, "Requested action range does not exist");
	}
}

shared_ptr<ActionOnVariable> MachineActuatorComponent::getAction(uint actionRank) const // Throws StatesException
{
	if (actionRank < (uint)this->actionList.count())
	{
		return this->actionList.at(actionRank);
	}
	else
	{
		throw StatesException("MachineActuatorComponent", MachineActuatorComponentError_t::out_of_range, "Requested action with rank #" + QString::number(actionRank) + " but only " + QString::number(this->actionList.count()) + " registered action(s).");
	}
}

QList<shared_ptr<ActionOnVariable> > MachineActuatorComponent::getActions() const
{
	return this->actionList;
}

void MachineActuatorComponent::changeActionRank(uint oldActionRank, uint newActionRank) // Throws StatesException
{
	if (oldActionRank < (uint)this->actionList.count())
	{
		if (newActionRank < (uint)this->actionList.count())
		{
			shared_ptr<ActionOnVariable> action = this->actionList.at(oldActionRank);
			this->actionList.removeAt(oldActionRank);
			this->actionList.insert(newActionRank, action);

			emit this->actionListChangedEvent();
			emit this->componentEditedEvent(this->id);
		}
		else
		{
			throw StatesException("MachineActuatorComponent", MachineActuatorComponentError_t::out_of_range, "Requested change action rank to rank #" + QString::number(newActionRank) + " but only " + QString::number(this->actionList.count()) + " registered action(s).");
		}
	}
	else
	{
		throw StatesException("MachineActuatorComponent", MachineActuatorComponentError_t::out_of_range, "Requested change action rank on action with rank #" + QString::number(oldActionRank) + " but only " + QString::number(this->actionList.count()) + " registered action(s).");
	}
}

void MachineActuatorComponent::cleanActionList()
{
	QList<shared_ptr<ActionOnVariable>> newActionList;

	bool listChanged = false;
	for (shared_ptr<ActionOnVariable> action : this->actionList)
	{
		if (action->getVariableActedOn() != nullptr)
		{
			newActionList.append(action);
		}
		else
		{
			listChanged = true;
		}
	}

	if (listChanged)
	{
		this->actionList = newActionList;
		emit this->actionListChangedEvent();
		emit this->componentEditedEvent(this->id);
	}
}

void MachineActuatorComponent::variableInActionListModifiedEventHandler()
{
	emit this->actionListChangedEvent();
	emit this->componentEditedEvent(this->id);
}
