/*
 * Copyright © 2014-2026 Clément Foucher
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

// StateS
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "actiononvariable.h"


std::shared_ptr<ActionOnVariable> MachineActuatorComponent::addAction(ComponentId targetVariableId)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto targetVariable = machine->getVariable(targetVariableId);
	if (targetVariable == nullptr) return nullptr;


	auto action = std::make_shared<ActionOnVariable>(targetVariableId, this->getAllowedActionBehaviors());

	this->addActionInternal(action, targetVariable);

	emit this->componentEditedEvent(this->id);

	return action;
}

void MachineActuatorComponent::addAction(std::shared_ptr<ActionOnVariable> action, std::shared_ptr<Variable> targetVariable)
{
	this->addActionInternal(action, targetVariable);
}

void MachineActuatorComponent::removeAction(uint actionRank)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	if (actionRank >= static_cast<uint>(this->actionList.count())) return;


	auto targetVariableId = actionList.at(actionRank)->getTargetVariableId();
	auto targetVariable = machine->getVariable(targetVariableId);
	if (targetVariable != nullptr)
	{
		disconnect(targetVariable.get(), &Variable::componentDeletedEvent, this, &MachineActuatorComponent::variableDeletedEventHandler);
	}

	this->actionList.removeAt(actionRank);

	emit this->componentEditedEvent(this->id);
}

std::shared_ptr<ActionOnVariable> MachineActuatorComponent::getAction(uint actionRank) const
{
	if (actionRank >= static_cast<uint>(this->actionList.count())) return nullptr;


	return this->actionList.at(actionRank);
}

const QList<std::shared_ptr<ActionOnVariable> > MachineActuatorComponent::getActions() const
{
	return this->actionList;
}

void MachineActuatorComponent::changeActionRank(uint oldActionRank, uint newActionRank)
{
	if (oldActionRank >= static_cast<uint>(this->actionList.count())) return;

	if (newActionRank >= static_cast<uint>(this->actionList.count())) return;

	if (oldActionRank == newActionRank) return;


	auto action = this->actionList.at(oldActionRank);
	this->actionList.removeAt(oldActionRank);
	this->actionList.insert(newActionRank, action);

	emit this->componentEditedEvent(this->id);
}

void MachineActuatorComponent::variableDeletedEventHandler(ComponentId deletedVariableId)
{
	QList<std::shared_ptr<ActionOnVariable>> newActionList;

	bool listChanged = false;
	for (auto& action : this->actionList)
	{
		if (action == nullptr)
		{
			listChanged = true;
			continue;
		}


		auto targetVariableId = action->getTargetVariableId();
		if (targetVariableId != deletedVariableId)
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

		emit this->componentEditedEvent(this->id);
	}
}

void MachineActuatorComponent::variableInActionListModifiedEventHandler()
{
	emit this->componentEditedEvent(this->id);
}

void MachineActuatorComponent::addActionInternal(std::shared_ptr<ActionOnVariable> action, std::shared_ptr<Variable> targetVariable)
{
	if (action == nullptr) return;

	if (targetVariable == nullptr) return;


	connect(action.get(), &ActionOnVariable::actionChangedEvent, this, &MachineActuatorComponent::variableInActionListModifiedEventHandler);
	// To remove destroyed variables from the action list
	// Use UniqueConnection as multiple actions on the same variable are possible due to actions on sub-vectors
	connect(targetVariable.get(), &Variable::componentDeletedEvent, this, &MachineActuatorComponent::variableDeletedEventHandler, Qt::UniqueConnection);

	connect(action.get(), &ActionOnVariable::actionChangedAsAResultOfAnotherChangeEvent, this, &MachineActuatorComponent::actionChangedAsAResultOfAnotherChangeEvent);

	this->actionList.append(action);
}
