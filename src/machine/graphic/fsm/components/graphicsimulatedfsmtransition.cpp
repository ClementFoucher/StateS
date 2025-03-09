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

// Qt classes
#include <QPen>

// States classes
#include "machinemanager.h"
#include "graphicfsm.h"
#include "simulatedfsm.h"
#include "simulatedfsmtransition.h"
#include "simulatedfsmstate.h"
#include "simulatedequation.h"
#include "graphicsimulatedfsmstate.h"


//
// Static elements
//

const QPen GraphicSimulatedFsmTransition::activePen   = QPen(QBrush(QColor(0, 0xB0, 0), Qt::SolidPattern), 3);
const QPen GraphicSimulatedFsmTransition::inactivePen = QPen(QBrush(Qt::red,            Qt::SolidPattern), 3);


//
// Class object definition
//

GraphicSimulatedFsmTransition::GraphicSimulatedFsmTransition(componentId_t logicComponentId) :
	GraphicFsmTransition(logicComponentId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto graphicTransition = graphicFsm->getTransition(this->logicComponentId);
	if (graphicTransition == nullptr) return;

	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto simulatedTransition = simulatedFsm->getSimulatedTransition(this->logicComponentId);
	if (simulatedTransition == nullptr) return;

	auto graphicSimulatedSourceState = graphicFsm->getSimulatedState(simulatedTransition->getSourceStateId());
	if (graphicSimulatedSourceState == nullptr) return;


	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsFocusable,  false);
	this->setFlag(QGraphicsItem::ItemClipsToShape, false);

	this->setAcceptHoverEvents(false);

	this->setConditionLineSliderPosition(graphicTransition->getConditionLineSliderPosition());

	connect(graphicSimulatedSourceState, &GraphicSimulatedFsmState::componentRefreshedEvent, this, &GraphicSimulatedFsmTransition::refreshDisplay);
}

void GraphicSimulatedFsmTransition::refreshDisplay()
{
	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto simulatedTransition = simulatedFsm->getSimulatedTransition(this->logicComponentId);
	if (simulatedTransition == nullptr) return;

	auto simulatedSourceState = simulatedFsm->getSimulatedState(simulatedTransition->getSourceStateId());
	if (simulatedSourceState == nullptr) return;


	//
	// Condition pen
	auto condition = simulatedTransition->getCondition();
	if (condition != nullptr)
	{
		if (condition->isTrue() == true)
		{
			this->currentConditionPen = &activePen;
		}
		else
		{
			this->currentConditionPen = &inactivePen;
		}
	}
	else
	{
		// Empty condition is implicitly always true
		this->currentConditionPen = &activePen;
	}

	//
	// Main pen
	if (simulatedSourceState->getIsActive() == true)
	{
		this->currentPen = this->currentConditionPen;
	}
	else
	{
		this->currentPen = &GraphicFsmTransition::defaultPen;
	}

	GraphicFsmTransition::refreshDisplay();
}
