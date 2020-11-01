/*
 * Copyright © 2014-2016 Clément Foucher
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
#include "StateS_signal.h"
#include "statesexception.h"
#include "actiononsignal.h"


MachineActuatorComponent::MachineActuatorComponent(shared_ptr<Machine> owningMachine) :
    MachineComponent(owningMachine)
{
	// Propagates list change events to the more general "configuration changed" event
	connect(this, &MachineActuatorComponent::actionListChangedEvent, this, &MachineComponent::componentNeedsGraphicUpdateEvent);
}

void MachineActuatorComponent::cleanActionList()
{
	QList<shared_ptr<ActionOnSignal>> newActionList;

	bool listChanged = false;
	foreach(shared_ptr<ActionOnSignal> action, this->actionList)
	{
		if (action->getSignalActedOn() != nullptr)
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
		emit actionListChangedEvent();
	}
}

QList<shared_ptr<ActionOnSignal> > MachineActuatorComponent::getActions() const
{
	return this->actionList;
}

shared_ptr<ActionOnSignal> MachineActuatorComponent::getAction(uint actionRank) const // Throws StatesException
{
	if (actionRank < (uint)this->actionList.count())
	{
		return this->actionList.at(actionRank);
	}
	else
	{
		throw StatesException("MachineActuatorComponent", out_of_range, "Requested action with rank #" + QString::number(actionRank) + " but only " + QString::number(this->actionList.count()) + " registered action(s).");
	}
}

shared_ptr<ActionOnSignal> MachineActuatorComponent::addAction(shared_ptr<Signal> signal)
{
	// Default action type
	ActionOnSignal::action_types actionType;

	if ((this->getAllowedActionTypes() & activeOnState) != 0)
	{
		actionType = ActionOnSignal::action_types::activeOnState;
	}
	else
	{
		actionType = ActionOnSignal::action_types::pulse;
	}


	shared_ptr<ActionOnSignal> action(new ActionOnSignal(signal, actionType));
	connect(action.get(), &ActionOnSignal::actionChangedEvent, this, &MachineActuatorComponent::actionListChangedEvent);
	this->actionList.append(action);

	// To remove destroyed signals from the action list
	connect(signal.get(), &Signal::signalDeletedEvent, this, &MachineActuatorComponent::cleanActionList);

	emit actionListChangedEvent();

	return action;
}

void MachineActuatorComponent::removeAction(uint actionRank) // Throws StatesException
{
	if (actionRank < (uint)this->actionList.count())
	{
		shared_ptr<Signal> signal = actionList.at(actionRank)->getSignalActedOn();
		if (signal != nullptr)
		{
			disconnect(signal.get(), &Signal::signalDeletedEvent, this, &MachineActuatorComponent::cleanActionList);
		}

		this->actionList.removeAt(actionRank);
		emit actionListChangedEvent();
	}
	else
	{
		throw StatesException("MachineActuatorComponent", out_of_range, "Requested action range does not exist");
	}
}

void MachineActuatorComponent::changeActionRank(uint oldActionRank, uint newActionRank) // Throws StatesException
{
	if (oldActionRank < (uint)this->actionList.count())
	{
		if (newActionRank < (uint)this->actionList.count())
		{
			shared_ptr<ActionOnSignal> action = this->actionList.at(oldActionRank);
			this->actionList.removeAt(oldActionRank);
			this->actionList.insert(newActionRank, action);

			emit actionListChangedEvent();
		}
		else
		{
			throw StatesException("MachineActuatorComponent", out_of_range, "Requested change action rank to rank #" + QString::number(newActionRank) + " but only " + QString::number(this->actionList.count()) + " registered action(s).");
		}
	}
	else
	{
		throw StatesException("MachineActuatorComponent", out_of_range, "Requested change action rank on action with rank #" + QString::number(oldActionRank) + " but only " + QString::number(this->actionList.count()) + " registered action(s).");
	}
}

void MachineActuatorComponent::activateActions()
{
	foreach (shared_ptr<ActionOnSignal> action, this->actionList)
	{
		action->beginAction();
	}
}

void MachineActuatorComponent::deactivateActions()
{
	foreach (shared_ptr<ActionOnSignal> action, this->actionList)
	{
		action->endAction();
	}
}
