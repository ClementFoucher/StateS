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
#include "actionbox.h"

// Qt
#include <QBrush>
#include <QPen>
#include <QGraphicsItem>

// StateS
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "variable.h"
#include "actiononvariable.h"
#include "equation.h"


//
// Static elements
//

const QPen ActionBox::defaultPen   = QPen(Qt::SolidPattern, 3);
const QPen ActionBox::conditionPen = QPen(Qt::SolidPattern, 3, Qt::DotLine);


//
// Class object definition
//

ActionBox::ActionBox(ComponentId actuatorId, bool addLine)
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
	qreal maxActionWidth = 0;
	qreal boxLeft = (this->addLine == true) ? 20 : 0;
	QList<QGraphicsItem*> itemsToAlignRight;

	for (int i = 0 ; i < actions.count() ; i++)
	{
		auto currentAction = actions[i];
		if (currentAction == nullptr) continue;

		auto currentVariableId = currentAction->getVariableActedOnId();

		auto currentVariable = machine->getVariable(currentVariableId);
		if (currentVariable == nullptr) continue;

		if (currentVariable->getInitialValue().isNull() == true) continue;


		qreal xPos = boxLeft;
		qreal currentActionWidth = 0;

		// Memorized state

		if (currentVariable->getMemorized() == true)
		{
			auto memorizedText = new QGraphicsTextItem(this);
			memorizedText->setHtml("<span style=\"color:black;\">M</span>");
			qreal memorizedTextWidth = memorizedText->boundingRect().width();

			memorizedText->setPos(QPointF(xPos, i*this->textHeight));
			memorizedText->setZValue(1);

			this->addToGroup(memorizedText);

			QPainterPath memorizedBorderPath;
			memorizedBorderPath.lineTo(0,                  this->textHeight);
			memorizedBorderPath.lineTo(memorizedTextWidth, this->textHeight);
			memorizedBorderPath.lineTo(memorizedTextWidth, 0);
			memorizedBorderPath.lineTo(0,                  0);

			auto memorizedOutline = new QGraphicsPathItem(memorizedBorderPath, this);
			memorizedOutline->setPen(ActionBox::defaultPen);
			memorizedOutline->setZValue(1);
			memorizedOutline->setPos(xPos, i*this->textHeight);

			currentActionWidth += memorizedTextWidth;
			xPos += memorizedText->boundingRect().width();
		}

		QString currentActionText = "<span style=\"color:black;\">";

		// Variable name
		currentActionText += currentVariable->getName();

		// Variable range
		if ( (currentVariable->getType() == MachineValue::Type_t::bitVector) && (currentVariable->getInitialValue().getBitVectorValue().getSize() > 1) )
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
		case ActionOnVariable::Type_t::set:
		case ActionOnVariable::Type_t::reset:
		case ActionOnVariable::Type_t::assign:
			currentActionText += " ← " + currentAction->getActionValue().toDisplayString();
			break;
		case ActionOnVariable::Type_t::continuous:
		case ActionOnVariable::Type_t::pulse:
			currentActionText += " ← " + currentAction->getActionValue().toDisplayString();
			break;
		case ActionOnVariable::Type_t::increment:
			currentActionText += " ← " + currentVariable->getName() + " + 1";
			break;
		case ActionOnVariable::Type_t::decrement:
			currentActionText += " ← " + currentVariable->getName() + " - 1";
			break;
		case ActionOnVariable::Type_t::none:
			// Nothing
			break;
		}

		currentActionText += "</span>";

		auto actionText = new QGraphicsTextItem(this);
		actionText->setHtml(currentActionText);
		qreal currentActionTextWidth = actionText->boundingRect().width();

		actionText->setPos(QPointF(xPos, i*this->textHeight));
		actionText->setZValue(1);

		this->addToGroup(actionText);

		currentActionWidth += currentActionTextWidth;

		// Action condition
		auto condition = currentAction->getActionCondition();
		if (condition != nullptr)
		{
			QString foregroundColor;
			if (condition->getType() == MachineValue::Type_t::boolean)
			{
				// Ok
				foregroundColor = "black";
			}
			else if ( (condition->getType() == MachineValue::Type_t::bitVector) && (condition->getInitialValue().getBitVectorValue().getSize() == 1) )
			{
				// Ok
				foregroundColor = "black";
			}
			else
			{
				// Condition is either valid but neither a boolean
				// nor a bit vector of size 1, or invalid.
				foregroundColor = "red";
			}

			QString conditionRawText;
			conditionRawText += "<span style=\"color:" + foregroundColor + ";\">";
			conditionRawText += "<i>";
			conditionRawText += tr("if");
			conditionRawText += "</i>";
			conditionRawText += " ";
			conditionRawText += condition->getText();
			conditionRawText += "</span>";

			auto conditionText = new QGraphicsTextItem(this);
			conditionText->setHtml(conditionRawText);
			qreal conditionTextWidth = conditionText->boundingRect().width();

			conditionText->setPos(QPointF(0, i*this->textHeight));
			conditionText->setZValue(1);

			this->addToGroup(conditionText);

			QPainterPath conditionBorderPath;
			conditionBorderPath.lineTo(0,                  this->textHeight);
			conditionBorderPath.lineTo(conditionTextWidth, this->textHeight);
			conditionBorderPath.lineTo(conditionTextWidth, 0);
			conditionBorderPath.lineTo(0,                  0);

			auto conditionOutline = new QGraphicsPathItem(conditionBorderPath, this);
			conditionOutline->setPen(ActionBox::conditionPen);
			conditionOutline->setZValue(1);
			conditionOutline->setPos(0, i*this->textHeight);

			itemsToAlignRight.append(conditionText);
			itemsToAlignRight.append(conditionOutline);
		}

		if (maxActionWidth < currentActionWidth)
		{
			maxActionWidth = currentActionWidth;
		}
	}

	for (auto item : itemsToAlignRight)
	{
		qreal x = boxLeft + maxActionWidth;
		qreal y = item->pos().y();
		item->setPos(x, y);
	}

	QPainterPath actionBorderPath;
	actionBorderPath.lineTo(0,              static_cast<qreal>(actions.count())*this->textHeight);
	actionBorderPath.lineTo(maxActionWidth, static_cast<qreal>(actions.count())*this->textHeight);
	actionBorderPath.lineTo(maxActionWidth, 0);
	actionBorderPath.lineTo(0,              0);

	this->actionsOutline = new QGraphicsPathItem(actionBorderPath, this);
	this->actionsOutline->setPen(ActionBox::defaultPen);
	this->actionsOutline->setBrush(QBrush(Qt::white, Qt::Dense3Pattern));
	this->actionsOutline->setZValue(0);
	this->actionsOutline->setPos(boxLeft, 0);

	this->addToGroup(this->actionsOutline);

	if (this->addLine == true)
	{
		qreal center = this->actionsOutline->boundingRect().height() / 2;
		auto leftLine = new QGraphicsLineItem(0, center, 20, center, this);
		leftLine->setPen(ActionBox::defaultPen);
	}
}

qreal ActionBox::getHeight() const
{
	if (this->actionsOutline == nullptr) return 0;


	return this->actionsOutline->boundingRect().height();
}
