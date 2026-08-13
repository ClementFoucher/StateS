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
#include "fsm.h"

// StateS
#include "fsmstate.h"
#include "fsmtransition.h"
#include "equation.h"


void Fsm::finalizeLoading()
{
	Machine::finalizeLoading();

	// When machine has just been loaded, all equations are invalid:
	// recompute all transitions conditions.
	for (auto transitionId : this->getAllTransitionsIds())
	{
		auto transition = this->getTransition(transitionId);
		auto condition = transition->getCondition();

		if (condition != nullptr)
		{
			condition->doFullStackRecomputation();
		}
	}
}

ComponentId Fsm::addState(const QString& name, ComponentId id)
{
	// Clean name
	QString cleanedName = name;
	this->cleanName(cleanedName);

	// Make sure there is actually a name
	if (cleanedName.isEmpty() == true) return nullId;

	// Check if name doesn't already exist
	for (auto existingStateId : as_const(this->states))
	{
		auto existingState = this->getState(existingStateId);
		if (existingState->getName() == cleanedName)
		{
			return nullId;
		}
	}


	// Build state
	shared_ptr<FsmState> state = nullptr;
	ComponentId stateId;
	if (id != nullId)
	{
		state = make_shared<FsmState>(id, cleanedName);
		stateId = id;
	}
	else
	{
		state = make_shared<FsmState>(cleanedName);
		stateId = state->getId();
	}

	this->states.append(stateId);
	this->registerComponent(state);

	return stateId;
}

ComponentId Fsm::addTransition(ComponentId sourceStateId, ComponentId targetStateId, ComponentId id)
{
	auto source = this->getState(sourceStateId);
	auto target = this->getState(targetStateId);

	if ( (source == nullptr) || (target == nullptr) ) return nullId;


	shared_ptr<FsmTransition> transition;
	ComponentId transitionId;

	if (id != nullId)
	{
		transition = make_shared<FsmTransition>(id, source->getId(), target->getId());
		transitionId = id;
	}
	else
	{
		transition = make_shared<FsmTransition>(source->getId(), target->getId());
		transitionId = transition->getId();
	}

	source->addOutgoingTransitionId(transitionId);
	target->addIncomingTransitionId(transitionId);

	this->transitions.append(transitionId);
	this->registerComponent(transition);

	return transitionId;
}

void Fsm::removeState(ComponentId stateId)
{
	auto state = this->getState(stateId);
	if (state == nullptr) return;


	for (auto transitionId : state->getOutgoingTransitionsIds())
	{
		this->removeTransition(transitionId);
	}

	for (auto transitionId : state->getIncomingTransitionsIds())
	{
		this->removeTransition(transitionId);
	}

	this->states.removeAll(stateId);
	this->removeComponent(stateId);
}

void Fsm::removeTransition(ComponentId transitionId)
{
	auto transition = this->getTransition(transitionId);
	if (transition == nullptr) return;


	auto sourceState = this->getState(transition->getSourceStateId());
	auto targetState = this->getState(transition->getTargetStateId());
	if ( (sourceState == nullptr) || (targetState == nullptr) ) return;

	sourceState->removeOutgoingTransitionId(transitionId);
	targetState->removeIncomingTransitionId(transitionId);

	this->transitions.removeAll(transitionId);
	this->removeComponent(transitionId);
}

shared_ptr<FsmState> Fsm::getState(ComponentId stateId) const
{
	return dynamic_pointer_cast<FsmState>(this->getComponent(stateId));
}

shared_ptr<FsmTransition> Fsm::getTransition(ComponentId transitionId) const
{
	return dynamic_pointer_cast<FsmTransition>(this->getComponent(transitionId));
}

const QList<ComponentId> Fsm::getAllStatesIds() const
{
	return this->states;
}

const QList<ComponentId> Fsm::getAllTransitionsIds() const
{
	return this->transitions;
}

bool Fsm::renameState(ComponentId stateId, const QString& newName)
{
	auto state = this->getState(stateId);
	if (state == nullptr) return false;

	// Clean name
	QString cleanedName = newName;
	this->cleanName(cleanedName);

	// If new name is identical to current name, nothing to do
	if (state->getName() == cleanedName) return true;

	// Make sure name is not empty
	if (cleanedName.isEmpty() == true) return false;

	// Check if name doesn't already exist
	for (auto otherStateId : as_const(this->states))
	{
		auto otherState = this->getState(otherStateId);
		if (otherState->getName() == cleanedName)
		{
			return false;
		}
	}


	// Set new name
	state->setName(cleanedName);

	return true;
}

void Fsm::redirectTransition(ComponentId transitionId, ComponentId newSourceStateId, ComponentId newTargetStateId)
{
	auto transition     = this->getTransition(transitionId);
	auto newSourceState = this->getState(newSourceStateId);
	auto newTargetState = this->getState(newTargetStateId);
	if ( (transition == nullptr) || (newSourceState == nullptr) || (newTargetState == nullptr) ) return;

	auto oldSourceStateId = transition->getSourceStateId();
	if (newSourceStateId != oldSourceStateId)
	{
		auto oldSourceState = this->getState(oldSourceStateId);
		if (oldSourceState != nullptr)
		{
			oldSourceState->removeOutgoingTransitionId(transition->getId());
		}

		transition->setSourceStateId(newSourceState->getId());
		newSourceState->addOutgoingTransitionId(transition->getId());
	}

	auto oldTargetStateId = transition->getTargetStateId();
	if (newTargetStateId != oldTargetStateId)
	{
		auto oldTargetState = this->getState(oldTargetStateId);
		if (oldTargetState != nullptr)
		{
			oldTargetState->removeIncomingTransitionId(transition->getId());
		}

		transition->setTargetStateId(newTargetState->getId());
		newTargetState->addIncomingTransitionId(transition->getId());
	}
}

void Fsm::setInitialState(ComponentId stateId)
{
	if (stateId == this->initialStateId) return;

	auto previousInitialState = this->getState(this->initialStateId);
	auto newInitialState      = this->getState(stateId);

	this->initialStateId = stateId;

	if (previousInitialState != nullptr)
	{
		emit this->componentEditedEvent(previousInitialState->getId());
	}
	if (newInitialState != nullptr)
	{
		emit this->componentEditedEvent(newInitialState->getId());
	}
}

ComponentId Fsm::getInitialStateId() const
{
	return this->initialStateId;
}
