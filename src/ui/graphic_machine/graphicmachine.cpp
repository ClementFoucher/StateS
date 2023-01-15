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
#include "graphicmachine.h"

// Qt classes
#include <QGraphicsItem>

// StateS classes
#include "machine.h"
#include "machinemanager.h"
#include "graphiccomponent.h"
#include "input.h"
#include "output.h"


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

	const qreal signalsLinesWidth = 20;
	const qreal horizontalSignalsNamesSpacer = 50;
	const qreal verticalElementsSpacer = 5;
	const qreal busesLineHeight = 10;
	const qreal busesLineWidth = 5;

	//
	// Draw inputs

	QGraphicsItemGroup* inputsGroup = new QGraphicsItemGroup();

	// Items position wrt. subgroup:
	// All items @ Y = 0, and rising
	// Signals names @ X > 0
	// Lines @ X < 0

	QList<shared_ptr<Input>> inputs = machine->getInputs();

	qreal currentInputY = 0;
	for(int i = 0 ; i < inputs.count() ; i++)
	{
		QGraphicsTextItem* textItem = new QGraphicsTextItem();

		QString text = "<span style=\"color:black;\">" + inputs[i]->getText() + "</span>";
		textItem->setHtml(text);

		inputsGroup->addToGroup(textItem);
		textItem->setPos(0, currentInputY);

		qreal currentLineY = currentInputY + textItem->boundingRect().height()/2;
		inputsGroup->addToGroup(new QGraphicsLineItem(-signalsLinesWidth, currentLineY, 0, currentLineY));//, inputsGroup);

		if (inputs[i]->getSize() > 1)
		{
			inputsGroup->addToGroup(new QGraphicsLineItem(-signalsLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, -signalsLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
			QGraphicsTextItem* sizeText = new QGraphicsTextItem(QString::number(inputs[i]->getSize()));
			inputsGroup->addToGroup(sizeText);
			sizeText->setPos(-signalsLinesWidth/2 - sizeText->boundingRect().width(), currentLineY - sizeText->boundingRect().height());
		}

		currentInputY += textItem->boundingRect().height();
	}

	//
	// Draw outputs

	QGraphicsItemGroup* outputsGroup = new QGraphicsItemGroup();

	// Items position wrt. subgroup:
	// All items @ Y = 0, and rising
	// Signals names @ X < 0
	// Lines @ X > 0

	QList<shared_ptr<Output>> outputs = machine->getOutputs();

	qreal currentOutputY = 0;
	for(int i = 0 ; i < outputs.count() ; i++)
	{
		QGraphicsTextItem* textItem = new QGraphicsTextItem();

		QString text = "<span style=\"color:black;\">" + outputs[i]->getText() + "</span>";
		textItem->setHtml(text);

		outputsGroup->addToGroup(textItem);
		textItem->setPos(-textItem->boundingRect().width(), currentOutputY);

		qreal currentLineY = currentOutputY + textItem->boundingRect().height()/2;
		outputsGroup->addToGroup(new QGraphicsLineItem(0, currentLineY, signalsLinesWidth, currentLineY));

		if (outputs[i]->getSize() > 1)
		{
			outputsGroup->addToGroup(new QGraphicsLineItem(signalsLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, signalsLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
			QGraphicsTextItem* sizeText = new QGraphicsTextItem(QString::number(outputs[i]->getSize()));
			outputsGroup->addToGroup(sizeText);
			sizeText->setPos(signalsLinesWidth/2, currentLineY - sizeText->boundingRect().height());
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

	qreal inputsNamesWidth = inputsGroup->boundingRect().width() - signalsLinesWidth;
	qreal outputsNamesWidth = outputsGroup->boundingRect().width() - signalsLinesWidth;

	componentWidth = inputsNamesWidth + horizontalSignalsNamesSpacer + outputsNamesWidth;

	if (componentWidth <= title->boundingRect().width() + horizontalSignalsNamesSpacer)
	{
		componentWidth = title->boundingRect().width() + horizontalSignalsNamesSpacer;
	}

	// Height

	qreal maxSignalsHeight = max(inputsGroup->boundingRect().height(), outputsGroup->boundingRect().height());

	componentHeight =
	        verticalElementsSpacer +
	        title->boundingRect().height() +
	        verticalElementsSpacer +
	        maxSignalsHeight +
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

	qreal verticalSignalsNameOffset = title->boundingRect().bottom() + verticalElementsSpacer;

	qreal inoutsDeltaHeight = inputsGroup->boundingRect().height() - outputsGroup->boundingRect().height();
	qreal additionalInputsOffet  = (inoutsDeltaHeight > 0 ? 0 : -inoutsDeltaHeight/2);
	qreal additionalOutputsOffet = (inoutsDeltaHeight < 0 ? 0 : inoutsDeltaHeight/2);

	inputsGroup-> setPos(0,              verticalSignalsNameOffset + additionalInputsOffet);
	outputsGroup->setPos(componentWidth, verticalSignalsNameOffset + additionalOutputsOffet);

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

QList<GraphicComponent*> GraphicMachine::getGraphicComponents() const
{
	return this->map.values();
}
