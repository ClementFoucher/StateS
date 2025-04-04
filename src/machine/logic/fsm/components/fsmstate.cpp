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
#include "fsmstate.h"


FsmState::FsmState(const QString& name) :
	MachineActuatorComponent()
{
	this->name = name;
}

FsmState::FsmState(componentId_t id, const QString& name) :
	MachineActuatorComponent(id)
{
	this->name = name;
}

void FsmState::setName(const QString& newName)
{
	this->name = newName;
	emit this->stateRenamedEvent();
	emit this->componentEditedEvent(this->id);
}

QString FsmState::getName() const
{
	return this->name;
}

void FsmState::addOutgoingTransitionId(componentId_t transitionId)
{
	this->outputTransitionsIds.append(transitionId);
	// Do not emit componentEditedEvent signal as linked transition lists
	// are just used for easy access and not actually part of the state
}

void FsmState::removeOutgoingTransitionId(componentId_t transitionId)
{
	this->outputTransitionsIds.removeAll(transitionId);
	// Do not emit componentEditedEvent signal as linked transition lists
	// are just used for easy access and not actually part of the state
}

const QList<componentId_t> FsmState::getOutgoingTransitionsIds() const
{
	return this->outputTransitionsIds;
}

void FsmState::addIncomingTransitionId(componentId_t transitionId)
{
	this->inputTransitionsIds.append(transitionId);
	// Do not emit componentEditedEvent signal as linked transition lists
	// are just used for easy access and not actually part of the state
}

void FsmState::removeIncomingTransitionId(componentId_t transitionId)
{
	this->inputTransitionsIds.removeAll(transitionId);
	// Do not emit componentEditedEvent signal as linked transition lists
	// are just used for easy access and not actually part of the state
}

const QList<componentId_t> FsmState::getIncomingTransitionsIds() const
{
	return this->inputTransitionsIds;
}

uint FsmState::getAllowedActionTypes() const
{
	return ((uint)actuatorAllowedActionType_t::continuous |
	        (uint)actuatorAllowedActionType_t::set        |
	        (uint)actuatorAllowedActionType_t::reset      |
	        (uint)actuatorAllowedActionType_t::assign     |
	        (uint)actuatorAllowedActionType_t::increment  |
	        (uint)actuatorAllowedActionType_t::decrement
	       );
}
