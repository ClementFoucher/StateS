/*
 * Copyright © 2014-2023 Clément Foucher
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
#include "graphicactuator.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QBrush>
#include <QGraphicsItem>

// StateS classes
#include "machinemanager.h"
#include "machineactuatorcomponent.h"
#include "StateS_signal.h"
#include "actiononsignal.h"
#include "machine.h"


GraphicActuator::GraphicActuator(componentId_t logicComponentId) :
    GraphicComponent(logicComponentId)
{
	this->actionsBox = new QGraphicsItemGroup();
}

GraphicActuator::~GraphicActuator()
{
	delete actionsBox;
}

QGraphicsItemGroup* GraphicActuator::getActionsBox() const
{
	return this->actionsBox;
}

void GraphicActuator::buildActionsBox(const QPen& pen, bool center)
{
	// Clean
	qDeleteAll(actionsBox->childItems());

	// Check
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->logicComponentId));
	if (actuator == nullptr) return;

	auto actions = actuator->getActions();
	if (actions.count() == 0) return;

	// Build
	qreal textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();
	qreal maxTextWidth = 0;

	for (int i = 0 ; i < actions.count() ; i++)
	{
		shared_ptr<ActionOnSignal> currentAction = actions[i];
		shared_ptr<Signal> currentSignal = currentAction->getSignalActedOn();

		QGraphicsTextItem* actionText = new QGraphicsTextItem(currentSignal->getName(), actionsBox);

		QString currentActionText;

		auto currentMode = machineManager->getCurrentSimulationMode();

		// Signal name

		if (currentMode == SimulationMode_t::simulateMode)
			currentActionText = currentSignal->getColoredText(true);
		else
			currentActionText = currentSignal->getText();

		// Signal range

		if (currentSignal->getSize() > 1)
		{
			int rangeL = currentAction->getActionRangeL();
			int rangeR = currentAction->getActionRangeR();

			if (rangeL != -1)
			{
				currentActionText += "<span style=\"color:black;\">";
				currentActionText += "[";
				currentActionText += QString::number(rangeL);

				if (rangeR != -1)
				{
					currentActionText += "..";
					currentActionText += QString::number(rangeR);
				}

				currentActionText += "]";
				currentActionText += "</span>";
			}
		}

		// Action value

		switch (currentAction->getActionType())
		{
		case ActionOnSignalType_t::set:
		case ActionOnSignalType_t::reset:
		case ActionOnSignalType_t::assign:
			currentActionText += "<span style=\"color:black;\">";
			currentActionText += " ← " + currentAction->getActionValue().toString(); // + "<sub>b</sub>";
			currentActionText += "</span>";
			break;
		case ActionOnSignalType_t::activeOnState:
		case ActionOnSignalType_t::pulse:
			if (currentAction->getActionSize() > 1)
			{
				currentActionText += "<span style=\"color:black;\">";
				currentActionText += " ↷ " + currentAction->getActionValue().toString(); // + "<sub>b</sub>";
				currentActionText += "</span>";
			}
		}

		actionText->setHtml(currentActionText);

		if (maxTextWidth < actionText->boundingRect().width())
			maxTextWidth = actionText->boundingRect().width();

		if (center)
			actionText->setPos(0, -( ( (textHeight*actions.count()) / 2) ) + i*textHeight);
		else
			actionText->setPos(QPointF(0, i*actionText->boundingRect().height()));

		actionText->setZValue(1);
	}

	QPainterPath actionBorderPath;
	if (center)
	{
		actionBorderPath.lineTo(0,                 ((qreal)actions.count()/2)*textHeight);
		actionBorderPath.lineTo(0 + maxTextWidth,  ((qreal)actions.count()/2)*textHeight);
		actionBorderPath.lineTo(0 + maxTextWidth, -((qreal)actions.count()/2)*textHeight);
		actionBorderPath.lineTo(0,                -((qreal)actions.count()/2)*textHeight);
		actionBorderPath.lineTo(0,                0);
		actionBorderPath.lineTo(-20 ,             0);
	}
	else
	{
		actionBorderPath.lineTo(0,            ((qreal)actions.count())*textHeight);
		actionBorderPath.lineTo(maxTextWidth, ((qreal)actions.count())*textHeight);
		actionBorderPath.lineTo(maxTextWidth, 0);
		actionBorderPath.lineTo(0,            0);
	}

	QGraphicsPathItem* stateActionsOutline = new QGraphicsPathItem(actionBorderPath, actionsBox);
	stateActionsOutline->setPen(pen);
	stateActionsOutline->setBrush(QBrush(Qt::white, Qt::Dense3Pattern));
	stateActionsOutline->setZValue(0);
	stateActionsOutline->setPos(0, 0);
}
