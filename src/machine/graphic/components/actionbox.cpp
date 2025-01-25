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
#include "variable.h"
#include "actiononvariable.h"
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
		shared_ptr<ActionOnVariable> currentAction = actions[i];
		shared_ptr<Variable> currentVariable = currentAction->getVariableActedOn();
		if (currentVariable == nullptr) continue;

		QString currentActionText;
		qreal xPos = boxLeft;
		qreal currentTextWidth = 0;

		// Memorized state

		bool isMemorized;
		switch (currentAction->getActionType())
		{
		case ActionOnVariableType_t::set:
		case ActionOnVariableType_t::reset:
		case ActionOnVariableType_t::assign:
		case ActionOnVariableType_t::increment:
		case ActionOnVariableType_t::decrement:
			isMemorized = true;
			break;
		case ActionOnVariableType_t::activeOnState:
		case ActionOnVariableType_t::pulse:
			isMemorized = false;
			break;
		}

		if (isMemorized == true)
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

		// Variable name

		currentActionText = "<span style=\"color:black;\">";
		currentActionText += currentVariable->getText();
		currentActionText += "</span>";

		// Variable range

		if (currentVariable->getSize() > 1)
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
		case ActionOnVariableType_t::set:
		case ActionOnVariableType_t::reset:
		case ActionOnVariableType_t::assign:
			currentActionText += "<span style=\"color:black;\">";
			currentActionText += " ← " + currentAction->getActionValue().toString(); // + "<sub>b</sub>";
			currentActionText += "</span>";
			break;
		case ActionOnVariableType_t::activeOnState:
		case ActionOnVariableType_t::pulse:
			if (currentAction->getActionSize() > 1)
			{
				currentActionText += "<span style=\"color:black;\">";
				currentActionText += " ← " + currentAction->getActionValue().toString(); // + "<sub>b</sub>";
				currentActionText += "</span>";
			}
			break;
		case ActionOnVariableType_t::increment:
			currentActionText += "<span style=\"color:black;\">";
			currentActionText += " ← " + currentVariable->getName() + " + 1";
			currentActionText += "</span>";
			break;
		case ActionOnVariableType_t::decrement:
			currentActionText += "<span style=\"color:black;\">";
			currentActionText += " ← " + currentVariable->getName() + " - 1";
			currentActionText += "</span>";
			break;
		}

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
