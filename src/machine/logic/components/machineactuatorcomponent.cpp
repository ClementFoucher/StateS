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

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "actiononvariable.h"


shared_ptr<ActionOnVariable> MachineActuatorComponent::addAction(componentId_t variableId)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return nullptr;


	auto action = make_shared<ActionOnVariable>(variableId, this->getAllowedActionTypes());

	this->addActionInternal(action, variable);

	emit this->actionListChangedEvent();
	emit this->componentEditedEvent(this->id);

	return action;
}

void MachineActuatorComponent::addAction(shared_ptr<ActionOnVariable> action, shared_ptr<Variable> variable)
{
	this->addActionInternal(action, variable);
}

void MachineActuatorComponent::removeAction(uint actionRank)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	if (actionRank >= (uint)this->actionList.count()) return;


	auto variableId = actionList.at(actionRank)->getVariableActedOnId();
	auto variable = machine->getVariable(variableId);
	if (variable != nullptr)
	{
		disconnect(variable.get(), &Variable::componentDeletedEvent, this, &MachineActuatorComponent::variableDeletedEventHandler);
	}

	this->actionList.removeAt(actionRank);

	emit this->actionListChangedEvent();
	emit this->componentEditedEvent(this->id);
}

shared_ptr<ActionOnVariable> MachineActuatorComponent::getAction(uint actionRank) const
{
	if (actionRank >= (uint)this->actionList.count()) return nullptr;


	return this->actionList.at(actionRank);
}

const QList<shared_ptr<ActionOnVariable> > MachineActuatorComponent::getActions() const
{
	return this->actionList;
}

void MachineActuatorComponent::changeActionRank(uint oldActionRank, uint newActionRank)
{
	if (oldActionRank >= (uint)this->actionList.count()) return;

	if (newActionRank >= (uint)this->actionList.count()) return;

	if (oldActionRank == newActionRank) return;


	shared_ptr<ActionOnVariable> action = this->actionList.at(oldActionRank);
	this->actionList.removeAt(oldActionRank);
	this->actionList.insert(newActionRank, action);

	emit this->actionListChangedEvent();
	emit this->componentEditedEvent(this->id);
}

void MachineActuatorComponent::variableDeletedEventHandler(componentId_t deletedVariableId)
{
	QList<shared_ptr<ActionOnVariable>> newActionList;

	bool listChanged = false;
	for (auto& action : this->actionList)
	{
		if (action == nullptr)
		{
			listChanged = true;
			continue;
		}


		auto variableId = action->getVariableActedOnId();
		if (variableId != deletedVariableId)
		{
			newActionList.append(action);
		}
		else // (variableId == deletedVariableId)
		{
			listChanged = true;
		}
	}

	if (listChanged == true)
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

void MachineActuatorComponent::addActionInternal(shared_ptr<ActionOnVariable> action, shared_ptr<Variable> variable)
{
	if (action == nullptr) return;

	if (variable == nullptr) return;


	connect(action.get(), &ActionOnVariable::actionChangedEvent, this, &MachineActuatorComponent::variableInActionListModifiedEventHandler);
	// To remove destroyed variables from the action list
	// Use UniqueConnection as multiple actions on the same variable are possible due to actions on sub-vectors
	connect(variable.get(), &Variable::componentDeletedEvent, this, &MachineActuatorComponent::variableDeletedEventHandler, Qt::UniqueConnection);

	this->actionList.append(action);
}
