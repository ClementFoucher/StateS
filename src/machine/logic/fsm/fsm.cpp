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
#include "fsm.h"

// StateS classes
#include "fsmstate.h"
#include "fsmtransition.h"


Fsm::Fsm() :
    Machine()
{

}

componentId_t Fsm::addState(bool isInitial, QString name, componentId_t id)
{
	shared_ptr<FsmState> state = nullptr;
	componentId_t stateId;

	if (id != nullId)
	{
		state = shared_ptr<FsmState>(new FsmState(id, this->getUniqueStateName(name)));
		stateId = id;
	}
	else
	{
		state = shared_ptr<FsmState>(new FsmState(this->getUniqueStateName(name)));
		stateId = state->getId();
	}

	this->states.append(stateId);
	this->registerComponent(state);

	if (isInitial)
	{
		this->setInitialState(stateId);
	}

	return stateId;
}

componentId_t Fsm::addTransition(componentId_t sourceStateId, componentId_t targetStateId, componentId_t id)
{
	auto source = this->getState(sourceStateId);
	auto target = this->getState(targetStateId);

	if ( (source == nullptr) || (target == nullptr) ) return nullId;

	shared_ptr<FsmTransition> transition;
	componentId_t transitionId;

	if (id != nullId)
	{
		transition = shared_ptr<FsmTransition>(new FsmTransition(id, source->getId(), target->getId()));
		transitionId = id;
	}
	else
	{
		transition = shared_ptr<FsmTransition>(new FsmTransition(source->getId(), target->getId()));
		transitionId = transition->getId();
	}

	source->addOutgoingTransitionId(transitionId);
	target->addIncomingTransitionId(transitionId);

	this->transitions.append(transitionId);
	this->registerComponent(transition);

	return transitionId;
}

void Fsm::removeState(componentId_t stateId)
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

void Fsm::removeTransition(componentId_t transitionId)
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

shared_ptr<FsmState> Fsm::getState(componentId_t stateId) const
{
	return dynamic_pointer_cast<FsmState>(this->getComponent(stateId));
}

shared_ptr<FsmTransition> Fsm::getTransition(componentId_t transitionId) const
{
	return dynamic_pointer_cast<FsmTransition>(this->getComponent(transitionId));
}

QList<componentId_t> Fsm::getAllStatesIds() const
{
	return this->states;
}

QList<componentId_t> Fsm::getAllTransitionsIds() const
{
	return this->transitions;
}

bool Fsm::renameState(componentId_t stateId, const QString& newName)
{
	auto state = this->getState(stateId);
	if (state == nullptr) return false;

	QString cleanedName = newName.trimmed();

	if (state->getName() == cleanedName)
	{
		// Nothing to do
		return true;
	}

	// By this point, we know the new name is at least different from current
	QString actualName = getUniqueStateName(cleanedName);

	if (actualName == cleanedName)
	{
		state->setName(actualName);
		return true;
	}
	else
	{
		return false;
	}
}

void Fsm::redirectTransition(componentId_t transitionId, componentId_t newSourceStateId, componentId_t newTargetStateId)
{
	auto transition     = this->getTransition(transitionId);
	auto newSourceState = this->getState(newSourceStateId);
	auto newTargetState = this->getState(newTargetStateId);
	if ( (transition == nullptr) || (newSourceState == nullptr) || (newTargetState == nullptr) ) return;

	componentId_t oldSourceStateId = transition->getSourceStateId();
	if (newSourceStateId != oldSourceStateId)
	{
		shared_ptr<FsmState> oldSourceState = this->getState(oldSourceStateId);
		if (oldSourceState != nullptr)
		{
			oldSourceState->removeOutgoingTransitionId(transition->getId());
		}

		transition->setSourceStateId(newSourceState->getId());
		newSourceState->addOutgoingTransitionId(transition->getId());
	}

	componentId_t oldTargetStateId = transition->getTargetStateId();
	if (newTargetStateId != oldTargetStateId)
	{
		shared_ptr<FsmState> oldTargetState = this->getState(oldTargetStateId);
		if (oldTargetState != nullptr)
		{
			oldTargetState->removeIncomingTransitionId(transition->getId());
		}

		transition->setTargetStateId(newTargetState->getId());
		newTargetState->addIncomingTransitionId(transition->getId());
	}
}

void Fsm::setInitialState(componentId_t stateId)
{
	if (stateId == this->initialStateId) return;

	shared_ptr<FsmState> previousInitialState = this->getState(this->initialStateId);
	shared_ptr<FsmState> newInitialState      = this->getState(stateId);

	this->initialStateId = stateId;

	if (previousInitialState != nullptr)
	{
		emit this->graphicComponentNeedsRefreshEvent(previousInitialState->getId());
	}
	if (newInitialState != nullptr)
	{
		emit this->graphicComponentNeedsRefreshEvent(newInitialState->getId());
	}
}

componentId_t Fsm::getInitialStateId() const
{
	return this->initialStateId;
}

QString Fsm::getUniqueStateName(QString nameProposal)
{
	QString baseName;
	QString currentName;
	uint i;

	if (nameProposal.isEmpty())
	{
		baseName = tr("State");
		currentName = baseName + " #0";
		i = 0;
	}
	else
	{
		baseName = nameProposal;
		currentName = nameProposal;
		i = 1;
	}

	bool nameIsValid = false;
	while (!nameIsValid)
	{
		nameIsValid = true;
		for (auto colleagueId : this->states)
		{
			auto colleague = this->getState(colleagueId);
			if (colleague->getName() == currentName)
			{
				nameIsValid = false;
				i++;
				currentName = baseName + " #" + QString::number(i);
				break;
			}
		}
	}

	return currentName;
}
