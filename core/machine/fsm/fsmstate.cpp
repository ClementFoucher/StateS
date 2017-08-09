/*
 * Copyright © 2014-2017 Clément Foucher
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

// StateS classes
#include "fsm.h"
#include "fsmgraphicstate.h"


FsmState::FsmState(shared_ptr<Fsm> parent, const QString& name, QPointF location) :
    FsmComponent(parent)
{
	this->name = name;

	this->graphicRepresentation = new FsmGraphicState();
	this->graphicRepresentation->setPos(location);

	connect(this->graphicRepresentation, &FsmGraphicState::stateMovedEvent, this, &FsmState::graphicRepresentationMovedEventHandler);
	connect(this->graphicRepresentation, &QObject::destroyed,               this, &FsmState::graphicRepresentationDeletedEventHandler);

	// Propagates local events to the more general events
	connect(this, &FsmState::stateRenamedEvent,               this, &MachineComponent::componentNeedsGraphicUpdateEvent);
	connect(this, &FsmState::stateSimulatedStateChangedEvent, this, &MachineComponent::componentSimulatedStateChangedEvent);
}

FsmState::~FsmState()
{
	delete this->graphicRepresentation;
}

FsmGraphicState* FsmState::getGraphicRepresentation()
{
	if (this->graphicRepresentation != nullptr)
	{
		this->graphicRepresentation->setLogicState(this->shared_from_this());
	}
	return this->graphicRepresentation;
}

uint FsmState::getAllowedActionTypes() const
{
	return (activeOnState | set | reset | assign);
}

/**
 * @brief FsmState::graphicRepresentationDeleted
 * As graphic state is handled by a scene, it can
 * be destroyed at any time. Clear obsolete reference
 * if so to avoid errors, notably at object deletion.
 */
void FsmState::graphicRepresentationDeletedEventHandler()
{
	this->graphicRepresentation = nullptr;
}

void FsmState::graphicRepresentationMovedEventHandler()
{
	emit statePositionChangedEvent(this->shared_from_this());
}

QString FsmState::getName() const
{
	return this->name;
}

void FsmState::setName(const QString& newName)
{
	this->name = newName;
	emit stateRenamedEvent();
}

void FsmState::addOutgoingTransition(shared_ptr<FsmTransition> transition)
{
	this->outputTransitions.append(transition);
}

void FsmState::removeOutgoingTransition(shared_ptr<FsmTransition> transition)
{
	QList<weak_ptr<FsmTransition>> newList;
	foreach(weak_ptr<FsmTransition> oldTransition, this->outputTransitions)
	{
		shared_ptr<FsmTransition> l_oldTransition = oldTransition.lock();

		// Keep all transitions except the one being removed
		if (l_oldTransition != transition)
		{
			newList.append(oldTransition);
		}
	}
	this->outputTransitions = newList;
}

const QList<shared_ptr<FsmTransition>> FsmState::getOutgoingTransitions() const
{
	QList<shared_ptr<FsmTransition>> transitions;
	foreach(weak_ptr<FsmTransition> transition, this->outputTransitions)
	{
		shared_ptr<FsmTransition> l_transition = transition.lock();
		if (l_transition != nullptr)
		{
			transitions.append(l_transition);
		}
	}
	return transitions;
}

void FsmState::addIncomingTransition(shared_ptr<FsmTransition> transition)
{
	this->inputTransitions.append(transition);
}

void FsmState::removeIncomingTransition(shared_ptr<FsmTransition> transition)
{
	QList<weak_ptr<FsmTransition>> newList;
	foreach(weak_ptr<FsmTransition> oldTransition, this->inputTransitions)
	{
		shared_ptr<FsmTransition> l_oldTransition = oldTransition.lock();

		// Keep all transitions except the one being removed
		if (l_oldTransition != transition)
		{
			newList.append(oldTransition);
		}
	}
	this->inputTransitions = newList;
}

const QList<shared_ptr<FsmTransition>> FsmState::getIncomingTransitions() const
{
	QList<shared_ptr<FsmTransition>> transitions;
	foreach(weak_ptr<FsmTransition> transition, this->inputTransitions)
	{
		shared_ptr<FsmTransition> l_transition = transition.lock();
		if (l_transition != nullptr)
		{
			transitions.append(l_transition);
		}
	}
	return transitions;
}

bool FsmState::getIsActive() const
{
	return this->isActive;
}

void FsmState::setActive(bool value)
{
	this->isActive = value;

	// On enter state, activate actions
	if (value == true)
	{
		this->activateActions();
	}
	else
	{
		this->deactivateActions();
	}

	emit stateSimulatedStateChangedEvent();
}

bool FsmState::isInitial() const
{
	shared_ptr<Fsm> owningFsm = this->getOwningFsm();
	if (owningFsm != nullptr)
	{
		return (owningFsm->getInitialState().get() == this);
	}
	else
	{
		return false;
	}
}

void FsmState::setInitial()
{
	shared_ptr<Fsm> owningFsm = this->getOwningFsm();
	if (owningFsm != nullptr)
	{
		owningFsm->setInitialState(this->shared_from_this());

		if (this->graphicRepresentation != nullptr)
		{
			this->graphicRepresentation->rebuildRepresentation();
		}
	}
}

void FsmState::notifyNotInitialAnyMore()
{
	if (this->graphicRepresentation != nullptr)
	{
		this->graphicRepresentation->rebuildRepresentation();
	}
}
