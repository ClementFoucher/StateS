/*
 * Copyright © 2023 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "graphicfsm.h"

// C++ classes
#include <math.h>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "graphicattributes.h"
#include "fsmstate.h"
#include "fsmgraphicstate.h"
#include "fsmtransition.h"
#include "fsmgraphictransition.h"
#include "fsmscene.h"
#include "fsmgraphictransitionneighborhood.h"


GraphicFsm::GraphicFsm() :
    GraphicMachine()
{

}

void GraphicFsm::build(shared_ptr<GraphicAttributes> graphicAttributes)
{
	if (graphicAttributes == nullptr) return;

	this->buildStates(graphicAttributes);
	this->buildTransitions(graphicAttributes);
}

shared_ptr<GraphicAttributes> GraphicFsm::getGraphicAttributes() const
{
	auto machineConfiguration = shared_ptr<GraphicAttributes>(new GraphicAttributes());

	auto states = this->getStates();
	for (auto state : states)
	{
		auto id = state->getLogicComponentId();
		auto scenePos = state->scenePos();
		machineConfiguration->addAttribute(id, attribute_t("X", QString::number(scenePos.x())));
		machineConfiguration->addAttribute(id, attribute_t("Y", QString::number(scenePos.y())));
	}

	auto transitions = this->getTransitions();
	for (auto transition : transitions)
	{
		auto id = transition->getLogicComponentId();
		auto sliderPos = transition->getConditionLineSliderPosition()*100;
		machineConfiguration->addAttribute(id, attribute_t("SliderPos", QString::number(sliderPos)));
	}

	return machineConfiguration;
}

GenericScene* GraphicFsm::getGraphicScene() const
{
	return new FsmScene();
}

void GraphicFsm::removeGraphicComponent(componentId_t id)
{
	auto transition = this->getTransition(id);

	if (transition != nullptr)
	{
		this->removeTransitionFromNeighborhood(id);
	}

	GraphicMachine::removeGraphicComponent(id);
}

FsmGraphicState* GraphicFsm::addState(componentId_t logicStateId, QPointF position)
{
	FsmGraphicState* graphicState = new FsmGraphicState(logicStateId);
	graphicState->setPos(position);

	this->addComponent(graphicState);

	return graphicState;
}

FsmGraphicTransition* GraphicFsm::addTransition(componentId_t logicTransitionId, qreal sliderPos)
{
	FsmGraphicTransition* graphicTransition = new FsmGraphicTransition(logicTransitionId);
	graphicTransition->setConditionLineSliderPosition(sliderPos);

	this->addComponent(graphicTransition);
	this->addTransitionToNeighborhood(graphicTransition->getLogicComponentId());

	return graphicTransition;
}

QList<FsmGraphicState*> GraphicFsm::getStates() const
{
	QList<FsmGraphicState*> statesList;

	for (GraphicComponent* graphicComponent : this->getGraphicComponents())
	{
		FsmGraphicState* s = dynamic_cast<FsmGraphicState*>(graphicComponent);
		if (s != nullptr) statesList.append(s);
	}

	return statesList;
}

QList<FsmGraphicTransition*> GraphicFsm::getTransitions() const
{
	QList<FsmGraphicTransition*> transitionsList;

	for (GraphicComponent* graphicComponent : this->getGraphicComponents())
	{
		FsmGraphicTransition* t = dynamic_cast<FsmGraphicTransition*>(graphicComponent);
		if (t != nullptr) transitionsList.append(t);
	}

	return transitionsList;
}

FsmGraphicState* GraphicFsm::getState(componentId_t id) const
{
	auto state = this->getGraphicComponent(id);
	return dynamic_cast<FsmGraphicState*>(state);
}

FsmGraphicTransition* GraphicFsm::getTransition(componentId_t id) const
{
	auto transition = this->getGraphicComponent(id);
	return dynamic_cast<FsmGraphicTransition*>(transition);
}

int GraphicFsm::getTransitionRank(componentId_t transitionId) const
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return 0;

	auto graphicTransition = this->getTransition(transitionId);
	if (graphicTransition == nullptr) return 0;

	auto neighborhood = this->getTransitionNeighborhood(transitionId);
	if (neighborhood == nullptr) return 0;

	int transitionNumber = neighborhood->getTransitionNumber(graphicTransition);

	// Keys are:            0,  1, 2,  3, 4,  5, etc.
	// Associate ranks are: 1, -1, 2, -2, 3, -3, etc.

	// This is then: order = (key/2 + 1)*(-1)^(key)
	int rank = (pow(-1, transitionNumber) * (transitionNumber/2 + 1));
	// If there is an odd number of mates, allow rank to begin from 0 (straight line)
	if ((neighborhood->count() % 2) == 1)
	{
		// To do so, decrement positive keys => 1, -1, 2, -2, 3 will become 0, -1, 1, -2, 2
		if (rank > 0)
		{
			rank--;
		}
	}

	return rank;
}

shared_ptr<FsmGraphicTransitionNeighborhood> GraphicFsm::getTransitionNeighborhood(componentId_t transitionId) const
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return nullptr;

	auto graphicTransition = this->getTransition(transitionId);
	if (graphicTransition == nullptr) return nullptr;

	auto sourceStateId = graphicTransition->getSourceStateId();
	auto targetStateId = graphicTransition->getTargetStateId();

	auto stateId1 = min(sourceStateId, targetStateId);
	auto stateId2 = max(sourceStateId, targetStateId);

	// Get neighborhood if it exists
	shared_ptr<FsmGraphicTransitionNeighborhood> neighborhood;
	if (this->neighborhoods.contains(stateId1))
	{
		if (this->neighborhoods[stateId1].contains(stateId2))
		{
			neighborhood = this->neighborhoods[stateId1][stateId2];
		}
	}

	return neighborhood;
}

void GraphicFsm::buildStates(shared_ptr<GraphicAttributes> configuration)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicStatesIds = fsm->getAllStatesIds();
	for (auto logicStateId : logicStatesIds)
	{
		auto graphicAttributes = configuration->getAttributes(logicStateId);
		qreal x = 0;
		qreal y = 0;

		for (auto &property : graphicAttributes)
		{
			if (property.first == "X")
			{
				x = property.second.toDouble();
			}
			else if (property.first == "Y")
			{
				y = property.second.toDouble();
			}
		}

		this->addState(logicStateId, QPointF(x,y));
	}
}

void GraphicFsm::buildTransitions(shared_ptr<GraphicAttributes> configuration)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicTransitionsIds = fsm->getAllTransitionsIds();
	for (auto logicTransitionId : logicTransitionsIds)
	{
		auto graphicAttributes = configuration->getAttributes(logicTransitionId);
		qreal sliderPos = 0;

		for (auto &property : graphicAttributes)
		{
			if (property.first == "SliderPos")
			{
				sliderPos = property.second.toDouble();
			}
		}

		this->addTransition(logicTransitionId, sliderPos);
	}
}

void GraphicFsm::addTransitionToNeighborhood(componentId_t transitionId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicTransition = this->getTransition(transitionId);

	auto sourceStateId = graphicTransition->getSourceStateId();
	auto targetStateId = graphicTransition->getTargetStateId();

	auto stateId1 = min(sourceStateId, targetStateId);
	auto stateId2 = max(sourceStateId, targetStateId);

	// Get neighborhood if it exists
	shared_ptr<FsmGraphicTransitionNeighborhood> neighborhood;
	if (this->neighborhoods.contains(stateId1))
	{
		if (this->neighborhoods[stateId1].contains(stateId2))
		{
			neighborhood = this->neighborhoods[stateId1][stateId2];
		}
	}

	// Build neighborhood if it doesn't exist and a transition
	// between the two states already exists
	if (neighborhood == nullptr)
	{
		shared_ptr<FsmTransition> newFriend;

		// Transitions from source to trarget
		auto logicState1 = fsm->getState(stateId1);
		foreach (auto outgoingTransitionId, logicState1->getOutgoingTransitionsIds())
		{
			// Ignore current transition
			if (outgoingTransitionId == transitionId) continue;

			// Ignore transition which have no graphic representation yey
			if (this->getTransition(outgoingTransitionId) == nullptr) continue;

			auto outgoingTransition = fsm->getTransition(outgoingTransitionId);
			if (outgoingTransition->getTargetStateId() == stateId2)
			{
				newFriend = outgoingTransition;
				break;
			}
		}

		// Transitions from target to source
		if (newFriend == nullptr)
		{
			auto logicState2 = fsm->getState(stateId2);
			foreach (auto outgoingTransitionId, logicState2->getOutgoingTransitionsIds())
			{
				// Ignore current transition
				if (outgoingTransitionId == transitionId) continue;

				// Ignore transition which have no graphic representation yey
				if (this->getTransition(outgoingTransitionId) == nullptr) continue;

				auto outgoingTransition = fsm->getTransition(outgoingTransitionId);
				if (outgoingTransition->getTargetStateId() == stateId1)
				{
					newFriend = outgoingTransition;
					break;
				}
			}
		}

		// If a neighbor was found
		if (newFriend != nullptr)
		{
			// Build neighborhood and act as if it already existed
			neighborhood = shared_ptr<FsmGraphicTransitionNeighborhood>(new FsmGraphicTransitionNeighborhood(this->getState(stateId1), this->getState(stateId2)));

			if (this->neighborhoods.contains(stateId1) == false)
			{
				this->neighborhoods[stateId1] = QHash<componentId_t, shared_ptr<FsmGraphicTransitionNeighborhood>>();
			}
			this->neighborhoods[stateId1][stateId2] = neighborhood;

			// New friend was already in neighborhood
			auto previousNeighbor = this->getTransition(newFriend->getId());
			neighborhood->insertTransition(previousNeighbor);
		}
	}

	// If a neighborhood was found, add new transition
	if (neighborhood != nullptr)
	{
		auto newNeighbor = this->getTransition(transitionId);
		neighborhood->insertTransition(newNeighbor);

		for (auto transition : neighborhood->getTransitions())
		{
			transition->refreshDisplay();
		}
	}
}

void GraphicFsm::removeTransitionFromNeighborhood(componentId_t transitionId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicTransition = this->getTransition(transitionId);

	auto sourceStateId = graphicTransition->getSourceStateId();
	auto targetStateId = graphicTransition->getTargetStateId();

	auto stateId1 = min(sourceStateId, targetStateId);
	auto stateId2 = max(sourceStateId, targetStateId);

	// Get neighborhood if it exists
	shared_ptr<FsmGraphicTransitionNeighborhood> neighborhood;
	if (this->neighborhoods.contains(stateId1))
	{
		if (this->neighborhoods[stateId1].contains(stateId2))
		{
			neighborhood = this->neighborhoods[stateId1][stateId2];
		}
	}

	// If neighborhood doesn't exist, nothing to do
	if (neighborhood == nullptr) return;


	// Remove transition from neighborhood
	neighborhood->removeTransition(graphicTransition);

	// Check if neighborhood is still required
	if (neighborhood->count() > 1)
	{
		// Still required, refresh transitions from neighborhood
		for (auto transition : neighborhood->getTransitions())
		{
			transition->refreshDisplay();
		}
	}
	else
	{
		// Not required any more, delete neighborhood
		auto remainingTransition = neighborhood->getTransitions()[0];

		this->neighborhoods[stateId1].remove(stateId2);
		if (this->neighborhoods[stateId1].count() == 0)
		{
			this->neighborhoods.remove(stateId1);
		}

		remainingTransition->refreshDisplay();
	}
}
