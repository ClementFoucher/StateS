/*
 * Copyright © 2023-2025 Clément Foucher
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
#include "graphicmachine.h"

// Qt classes
#include <QGraphicsItem>

// StateS classes
#include "machine.h"
#include "machinemanager.h"
#include "graphiccomponent.h"
#include "variable.h"


GraphicMachine::GraphicMachine() :
    QObject()
{

}

GraphicMachine::~GraphicMachine()
{
	auto graphicComponents = this->getGraphicComponents();

	for (auto graphicComponent : graphicComponents)
	{
		delete graphicComponent;
	}
}

GraphicComponent* GraphicMachine::getGraphicComponent(componentId_t componentId) const
{
	if (this->map.contains(componentId))
	{
		return this->map[componentId];
	}
	else
	{
		return nullptr;
	}
}

QGraphicsItem* GraphicMachine::getComponentVisualization() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	// /!\ QGraphicsItemGroup bounding box seems not to be updated
	// if item is added using its constructor's parent parameter

	QGraphicsItemGroup* visu = new QGraphicsItemGroup();

	//
	// Main sizes

	const qreal variablesLinesWidth = 20;
	const qreal horizontalVariablesNamesSpacer = 50;
	const qreal verticalElementsSpacer = 5;
	const qreal busesLineHeight = 10;
	const qreal busesLineWidth = 5;

	//
	// Draw inputs

	QGraphicsItemGroup* inputsGroup = new QGraphicsItemGroup();

	// Items position wrt. subgroup:
	// All items @ Y = 0, and rising
	// Variables names @ X > 0
	// Lines @ X < 0

	qreal currentInputY = 0;
	for(auto& input : machine->getInputs())
	{
		QGraphicsTextItem* textItem = new QGraphicsTextItem();

		QString text = "<span style=\"color:black;\">" + input->getName() + "</span>";
		textItem->setHtml(text);

		inputsGroup->addToGroup(textItem);
		textItem->setPos(0, currentInputY);

		qreal currentLineY = currentInputY + textItem->boundingRect().height()/2;
		inputsGroup->addToGroup(new QGraphicsLineItem(-variablesLinesWidth, currentLineY, 0, currentLineY));

		if (input->getSize() > 1)
		{
			inputsGroup->addToGroup(new QGraphicsLineItem(-variablesLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, -variablesLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
			QGraphicsTextItem* sizeTextItem = new QGraphicsTextItem();
			QString textSize = "<span style=\"color:black;\">" + QString::number(input->getSize()) + "</span>";
			sizeTextItem->setHtml(textSize);
			inputsGroup->addToGroup(sizeTextItem);
			sizeTextItem->setPos(-variablesLinesWidth/2 - sizeTextItem->boundingRect().width(), currentLineY - sizeTextItem->boundingRect().height());
		}

		currentInputY += textItem->boundingRect().height();
	}

	//
	// Draw outputs

	QGraphicsItemGroup* outputsGroup = new QGraphicsItemGroup();

	// Items position wrt. subgroup:
	// All items @ Y = 0, and rising
	// Variables names @ X < 0
	// Lines @ X > 0

	qreal currentOutputY = 0;
	for(auto& output : machine->getOutputs())
	{
		QGraphicsTextItem* textItem = new QGraphicsTextItem();

		QString text = "<span style=\"color:black;\">" + output->getName() + "</span>";
		textItem->setHtml(text);

		outputsGroup->addToGroup(textItem);
		textItem->setPos(-textItem->boundingRect().width(), currentOutputY);

		qreal currentLineY = currentOutputY + textItem->boundingRect().height()/2;
		outputsGroup->addToGroup(new QGraphicsLineItem(0, currentLineY, variablesLinesWidth, currentLineY));

		if (output->getSize() > 1)
		{
			outputsGroup->addToGroup(new QGraphicsLineItem(variablesLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, variablesLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
			QGraphicsTextItem* sizeTextItem = new QGraphicsTextItem();
			QString textSize = "<span style=\"color:black;\">" + QString::number(output->getSize()) + "</span>";
			sizeTextItem->setHtml(textSize);
			outputsGroup->addToGroup(sizeTextItem);
			sizeTextItem->setPos(variablesLinesWidth/2, currentLineY - sizeTextItem->boundingRect().height());
		}

		currentOutputY += textItem->boundingRect().height();
	}

	//
	// Draw component name

	QGraphicsTextItem* title = new QGraphicsTextItem();
	title->setHtml("<span style=\"color:black; font-weight:bold;\">" + machine->getName() + "</span>");

	//
	// Compute component size

	qreal componentWidth;
	qreal componentHeight;

	// Width

	qreal inputsNamesWidth = inputsGroup->boundingRect().width() - variablesLinesWidth;
	qreal outputsNamesWidth = outputsGroup->boundingRect().width() - variablesLinesWidth;

	componentWidth = inputsNamesWidth + horizontalVariablesNamesSpacer + outputsNamesWidth;

	if (componentWidth <= title->boundingRect().width() + horizontalVariablesNamesSpacer)
	{
		componentWidth = title->boundingRect().width() + horizontalVariablesNamesSpacer;
	}

	// Height

	qreal maxVariablesHeight = max(inputsGroup->boundingRect().height(), outputsGroup->boundingRect().height());

	componentHeight =
	        verticalElementsSpacer +
	        title->boundingRect().height() +
	        verticalElementsSpacer +
	        maxVariablesHeight +
	        verticalElementsSpacer;

	//
	// Draw component border

	QGraphicsPolygonItem* border = nullptr;


	QPolygonF borderPolygon;
	borderPolygon.append(QPoint(0,              0));
	borderPolygon.append(QPoint(componentWidth, 0));
	borderPolygon.append(QPoint(componentWidth, componentHeight));
	borderPolygon.append(QPoint(0,              componentHeight));

	border = new QGraphicsPolygonItem(borderPolygon);

	//
	// Place components in main group

	// Items position wrt. main group:
	// Component top left corner @ (0; 0)

	visu->addToGroup(border);
	visu->addToGroup(title);
	visu->addToGroup(inputsGroup);
	visu->addToGroup(outputsGroup);

	border->setPos(0, 0);

	title->setPos( (componentWidth-title->boundingRect().width())/2, verticalElementsSpacer);

	qreal verticalVariablesNameOffset = title->boundingRect().bottom() + verticalElementsSpacer;

	qreal inoutsDeltaHeight = inputsGroup->boundingRect().height() - outputsGroup->boundingRect().height();
	qreal additionalInputsOffet  = (inoutsDeltaHeight > 0 ? 0 : -inoutsDeltaHeight/2);
	qreal additionalOutputsOffet = (inoutsDeltaHeight < 0 ? 0 : inoutsDeltaHeight/2);

	inputsGroup-> setPos(0,              verticalVariablesNameOffset + additionalInputsOffet);
	outputsGroup->setPos(componentWidth, verticalVariablesNameOffset + additionalOutputsOffet);

	//
	// Done

	return visu;
}

void GraphicMachine::removeGraphicComponent(componentId_t id)
{
	this->map[id]->deleteLater();
	this->map.remove(id);
}

void GraphicMachine::addComponent(GraphicComponent* graphicComponent)
{
	auto logicComponentId = graphicComponent->getLogicComponentId();
	this->map[logicComponentId] = graphicComponent;
}

const QList<GraphicComponent *> GraphicMachine::getGraphicComponents() const
{
	return this->map.values();
}
