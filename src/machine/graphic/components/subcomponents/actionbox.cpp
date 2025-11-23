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

// Qt classes
#include <QBrush>
#include <QPen>
#include <QGraphicsItem>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "variable.h"
#include "actiononvariable.h"


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

	// Check
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return;

	auto actions = actuator->getActions();
	if (actions.count() == 0) return;


	// Build
	qreal maxTextWidth = 0;
	qreal boxLeft = (this->addLine == true) ? 20 : 0;

	for (int i = 0 ; i < actions.count() ; i++)
	{
		auto currentAction = actions[i];
		if (currentAction == nullptr) continue;

		auto currentVariableId = currentAction->getVariableActedOnId();

		auto currentVariable = machine->getVariable(currentVariableId);
		if (currentVariable == nullptr) continue;


		qreal xPos = boxLeft;
		qreal currentTextWidth = 0;

		// Memorized state

		if (currentVariable->getMemorized() == true)
		{
			auto memorizedText = new QGraphicsTextItem(this);
			memorizedText->setHtml("<span style=\"color:black;\">M</span>");
			currentTextWidth += memorizedText->boundingRect().width();

			memorizedText->setPos(QPointF(xPos, i*this->textHeight));
			memorizedText->setZValue(1);

			this->addToGroup(memorizedText);

			QPainterPath memorizedBorderPath;
			memorizedBorderPath.lineTo(0,                this->textHeight);
			memorizedBorderPath.lineTo(currentTextWidth, this->textHeight);
			memorizedBorderPath.lineTo(currentTextWidth, 0);
			memorizedBorderPath.lineTo(0,                0);

			auto memorizedOutline = new QGraphicsPathItem(memorizedBorderPath, this);
			memorizedOutline->setPen(defaultPen);
			memorizedOutline->setZValue(1);
			memorizedOutline->setPos(xPos, i*this->textHeight);

			xPos += memorizedText->boundingRect().width();
		}


		QString currentActionText = "<span style=\"color:black;\">";

		// Variable name
		currentActionText += currentVariable->getName();

		// Variable range

		if (currentVariable->getSize() > 1)
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
		case ActionOnVariableType_t::set:
		case ActionOnVariableType_t::reset:
		case ActionOnVariableType_t::assign:
			currentActionText += " ← " + currentAction->getActionValue().toString();
			break;
		case ActionOnVariableType_t::continuous:
		case ActionOnVariableType_t::pulse:
			if (currentAction->getActionSize() > 1)
			{
				currentActionText += " ← " + currentAction->getActionValue().toString();
			}
			break;
		case ActionOnVariableType_t::increment:
			currentActionText += " ← " + currentVariable->getName() + " + 1";
			break;
		case ActionOnVariableType_t::decrement:
			currentActionText += " ← " + currentVariable->getName() + " - 1";
			break;
		case ActionOnVariableType_t::none:
			// Nothing
			break;
		}

		currentActionText += "</span>";

		QGraphicsTextItem* actionText = new QGraphicsTextItem(this);
		actionText->setHtml(currentActionText);
		currentTextWidth += actionText->boundingRect().width();

		if (maxTextWidth < currentTextWidth)
			maxTextWidth = currentTextWidth;

		actionText->setPos(QPointF(xPos, i*this->textHeight));
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
		auto leftLine = new QGraphicsLineItem(0, center, 20, center, this);
		leftLine->setPen(defaultPen);
	}
}

qreal ActionBox::getHeight() const
{
	if (this->actionsOutline == nullptr) return 0;


	return this->actionsOutline->boundingRect().height();
}
