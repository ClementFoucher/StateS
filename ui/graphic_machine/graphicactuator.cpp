/*
 * Copyright © 2014-2021 Clément Foucher
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

// Qt classes
#include <QBrush>
#include <QGraphicsItem>

// StateS classes
#include "machineactuatorcomponent.h"
#include "StateS_signal.h"
#include "machine.h"
#include "statesexception.h"
#include "actiononsignal.h"


GraphicActuator::GraphicActuator()
{
	this->initialize();
}

GraphicActuator::~GraphicActuator()
{
	delete actionsBox;
}

QGraphicsItemGroup* GraphicActuator::getActionsBox() const
{
	return this->actionsBox;
}

void GraphicActuator::setLogicActuator(shared_ptr<MachineActuatorComponent> actuator) // Throws StatesException
{
	this->setLogicComponent(dynamic_pointer_cast<MachineComponent>(actuator)); // Throws StatesException - propagated
}

shared_ptr<MachineActuatorComponent> GraphicActuator::getLogicActuator() const
{
	return dynamic_pointer_cast<MachineActuatorComponent>(this->getLogicComponent());
}

void GraphicActuator::buildActionsBox(const QPen& pen, bool center)
{
	// Clean
	qDeleteAll(actionsBox->childItems());

	// Check base reference
	shared_ptr<MachineActuatorComponent> l_actuator = this->getLogicActuator();
	if (l_actuator != nullptr)
	{
		// Begin build
		QList<shared_ptr<ActionOnSignal>> actions = l_actuator->getActions();
		if (actions.count() != 0)
		{
			qreal textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();
			qreal maxTextWidth = 0;

			for (int i = 0 ; i < actions.count() ; i++)
			{
				shared_ptr<ActionOnSignal> currentAction = actions[i];
				shared_ptr<Signal> currentSignal = currentAction->getSignalActedOn();

				QGraphicsTextItem* actionText = new QGraphicsTextItem(currentSignal->getName(), actionsBox);

				QString currentActionText;

				Machine::simulation_mode currentMode = l_actuator->getOwningMachine()->getCurrentSimulationMode();

				// Signal name

				if (currentMode == Machine::simulation_mode::simulateMode)
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
						currentActionText += "[";
						currentActionText += QString::number(rangeL);

						if (rangeR != -1)
						{
							currentActionText += "..";
							currentActionText += QString::number(rangeR);
						}

						currentActionText += "]";
					}
				}

				// Action value

				switch (currentAction->getActionType())
				{
				case ActionOnSignal::action_types::set:
				case ActionOnSignal::action_types::reset:
				case ActionOnSignal::action_types::assign:
					currentActionText += " ← " + currentAction->getActionValue().toString(); // + "<sub>b</sub>";
					break;
				case ActionOnSignal::action_types::activeOnState:
				case ActionOnSignal::action_types::pulse:
					if (currentAction->getActionSize() > 1)
					{
						currentActionText += " ↷ " + currentAction->getActionValue().toString(); // + "<sub>b</sub>";
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
	}
}

void GraphicActuator::initialize()
{
	this->actionsBox = new QGraphicsItemGroup();
}
