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
#include "actionbox.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QBrush>
#include <QPen>
#include <QGraphicsItem>

// StateS classes
#include "machinemanager.h"
#include "machineactuatorcomponent.h"
#include "StateS_signal.h"
#include "actiononsignal.h"
#include "machine.h"


//
// Static elements
//

const QPen ActionBox::defaultPen = QPen(Qt::SolidPattern, 3);


//
// Class object definition
//

ActionBox::ActionBox(componentId_t actuatorId, bool addLine)
{
	this->actuatorId = actuatorId;
	this->addLine = addLine;
	this->textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();

	this->buildActionBox();
}

void ActionBox::refreshDisplay()
{
	this->buildActionBox();
}

void ActionBox::buildActionBox()
{
	// Clean
	qDeleteAll(this->childItems());
	this->actionsOutline = nullptr;
	this->leftLine       = nullptr;

	// Check
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;

	auto actions = actuator->getActions();
	if (actions.count() == 0) return;

	// Build
	qreal maxTextWidth = 0;
	qreal boxLeft = (this->addLine == true) ? 20 : 0;

	for (int i = 0 ; i < actions.count() ; i++)
	{
		shared_ptr<ActionOnSignal> currentAction = actions[i];
		shared_ptr<Signal> currentSignal = currentAction->getSignalActedOn();

		QGraphicsTextItem* actionText = new QGraphicsTextItem(currentSignal->getName(), this);

		QString currentActionText;

		// Signal name

		currentActionText = "<span style=\"color:black;\">";
		currentActionText += currentSignal->getText();
		currentActionText += "</span>";


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
			break;
		case ActionOnSignalType_t::increment:
			currentActionText += "<span style=\"color:black;\">";
			currentActionText += " ← " + currentSignal->getName() + " + 1";
			currentActionText += "</span>";
			break;
		case ActionOnSignalType_t::decrement:
			currentActionText += "<span style=\"color:black;\">";
			currentActionText += " ← " + currentSignal->getName() + " - 1";
			currentActionText += "</span>";
			break;
		}

		actionText->setHtml(currentActionText);

		if (maxTextWidth < actionText->boundingRect().width())
			maxTextWidth = actionText->boundingRect().width();

		actionText->setPos(QPointF(boxLeft, i*actionText->boundingRect().height()));

		actionText->setZValue(1);

		this->addToGroup(actionText);
	}

	QPainterPath actionBorderPath;
	actionBorderPath.lineTo(0,            ((qreal)actions.count())*this->textHeight);
	actionBorderPath.lineTo(maxTextWidth, ((qreal)actions.count())*this->textHeight);
	actionBorderPath.lineTo(maxTextWidth, 0);
	actionBorderPath.lineTo(0,            0);

	this->actionsOutline = new QGraphicsPathItem(actionBorderPath, this);
	this->actionsOutline->setPen(defaultPen);
	this->actionsOutline->setBrush(QBrush(Qt::white, Qt::Dense3Pattern));
	this->actionsOutline->setZValue(0);
	this->actionsOutline->setPos(boxLeft, 0);

	this->addToGroup(this->actionsOutline);

	if (this->addLine == true)
	{
		qreal center = this->actionsOutline->boundingRect().height() / 2;
		this->leftLine = new QGraphicsLineItem(0, center, 20, center, this);
		this->leftLine->setPen(defaultPen);
	}
}

qreal ActionBox::getHeight() const
{
	if (this->actionsOutline == nullptr) return 0;


	return this->actionsOutline->boundingRect().height();
}
