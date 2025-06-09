/*
 * Copyright © 2025 Clément Foucher
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
#include "graphicsimulatedfsmtransition.h"

// States classes
#include "machinemanager.h"
#include "graphicfsm.h"
#include "simulatedfsm.h"
#include "simulatedfsmtransition.h"
#include "simulatedfsmstate.h"
#include "simulatedequation.h"
#include "graphicsimulatedfsmstate.h"


GraphicSimulatedFsmTransition::GraphicSimulatedFsmTransition(componentId_t logicComponentId) :
	GraphicFsmTransition(logicComponentId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto graphicTransition = graphicFsm->getTransition(this->getLogicComponentId());
	if (graphicTransition == nullptr) return;

	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto simulatedTransition = simulatedFsm->getSimulatedTransition(this->getLogicComponentId());
	if (simulatedTransition == nullptr) return;

	auto graphicSimulatedSourceState = graphicFsm->getSimulatedState(simulatedTransition->getSourceStateId());
	if (graphicSimulatedSourceState == nullptr) return;


	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsFocusable,  false);
	this->setFlag(QGraphicsItem::ItemClipsToShape, false);

	this->setAcceptHoverEvents(false);

	this->setConditionLineSliderPosition(graphicTransition->getConditionLineSliderPosition());

	connect(graphicSimulatedSourceState, &GraphicSimulatedFsmState::componentRefreshedEvent, this, &GraphicSimulatedFsmTransition::refreshSimulatedDisplay);
}

void GraphicSimulatedFsmTransition::refreshSimulatedDisplay()
{
	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto simulatedTransition = simulatedFsm->getSimulatedTransition(this->getLogicComponentId());
	if (simulatedTransition == nullptr) return;

	auto simulatedSourceState = simulatedFsm->getSimulatedState(simulatedTransition->getSourceStateId());
	if (simulatedSourceState == nullptr) return;


	// Determine new color
	QColor newColor;

	auto condition = simulatedTransition->getCondition();
	if (condition != nullptr)
	{
		if (condition->isTrue() == true)
		{
			newColor = GraphicSimulatedComponent::simuActiveBorderColor;
		}
		else
		{
			newColor = GraphicSimulatedComponent::simuInactiveBorderColor;
		}
	}
	else
	{
		// Empty condition is implicitly always true
		newColor = GraphicSimulatedComponent::simuActiveBorderColor;
	}

	// Set arrow pen
	if (simulatedSourceState->getIsActive() == true)
	{
		this->setArrowColor(newColor);
	}
	else
	{
		this->setArrowColor(GraphicComponent::defaultBorderColor);
	}

	// Set condition line pen
	this->setConditionColor(newColor);
}
