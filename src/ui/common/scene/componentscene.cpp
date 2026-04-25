/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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
#include "componentscene.h"

// Qt classes
#include <QGraphicsItem>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"


ComponentScene::ComponentScene()
{
	this->build();
}

void ComponentScene::build()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	//
	// Main sizes

	const qreal variablesLinesWidth            = 20;
	const qreal horizontalVariablesNamesSpacer = 50;
	const qreal verticalElementsSpacer         = 5;
	const qreal busesLineHeight                = 10;
	const qreal busesLineWidth                 = 5;

	//
	// Draw inputs

	auto inputsGroup = new QGraphicsItemGroup();

	// Items position wrt. group:
	// All items @ Y = 0, and rising
	// Variables names @ X > 0
	// Lines @ X < 0

	qreal currentInputY = 0;
	for (auto& inputId : machine->getInputVariablesIds())
	{
		auto input = machine->getVariable(inputId);
		if (input == nullptr) continue;


		auto textItem = this->buildGraphicsTextItem(input->getName());
		inputsGroup->addToGroup(textItem);
		textItem->setPos(0, currentInputY);

		qreal currentLineY = currentInputY + textItem->boundingRect().height()/2;
		inputsGroup->addToGroup(new QGraphicsLineItem(-variablesLinesWidth, currentLineY, 0, currentLineY));

		if (input->getSize() > 1)
		{
			inputsGroup->addToGroup(new QGraphicsLineItem(-variablesLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, -variablesLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));

			auto sizeTextItem = this->buildGraphicsTextItem(QString::number(input->getSize()));
			inputsGroup->addToGroup(sizeTextItem);
			sizeTextItem->setPos(-variablesLinesWidth/2 - sizeTextItem->boundingRect().width(), currentLineY - sizeTextItem->boundingRect().height());
		}

		currentInputY += textItem->boundingRect().height();
	}

	//
	// Draw outputs

	auto outputsGroup = new QGraphicsItemGroup();

	// Items position wrt. group:
	// All items @ Y = 0, and rising
	// Variables names @ X < 0
	// Lines @ X > 0

	qreal currentOutputY = 0;
	for (auto& outputId : machine->getOutputVariablesIds())
	{
		auto output = machine->getVariable(outputId);
		if (output == nullptr) continue;


		auto textItem = this->buildGraphicsTextItem(output->getName());
		outputsGroup->addToGroup(textItem);
		textItem->setPos(-textItem->boundingRect().width(), currentOutputY);

		qreal currentLineY = currentOutputY + textItem->boundingRect().height()/2;
		outputsGroup->addToGroup(new QGraphicsLineItem(0, currentLineY, variablesLinesWidth, currentLineY));

		if (output->getSize() > 1)
		{
			outputsGroup->addToGroup(new QGraphicsLineItem(variablesLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, variablesLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));

			auto sizeTextItem = this->buildGraphicsTextItem(QString::number(output->getSize()));
			outputsGroup->addToGroup(sizeTextItem);
			sizeTextItem->setPos(variablesLinesWidth/2, currentLineY - sizeTextItem->boundingRect().height());
		}

		currentOutputY += textItem->boundingRect().height();
	}

	//
	// Draw component name

	auto name = this->buildGraphicsTextItem("<span style=\"font-weight:bold;\">" + machine->getName() + "</span>");

	//
	// Compute component size

	// Width

	qreal inputsNamesWidth  = inputsGroup ->boundingRect().width() - variablesLinesWidth;
	qreal outputsNamesWidth = outputsGroup->boundingRect().width() - variablesLinesWidth;

	qreal componentWidth = inputsNamesWidth + horizontalVariablesNamesSpacer + outputsNamesWidth;

	if (componentWidth <= name->boundingRect().width() + horizontalVariablesNamesSpacer)
	{
		componentWidth = name->boundingRect().width() + horizontalVariablesNamesSpacer;
	}

	// Height

	qreal maxVariablesHeight = max(inputsGroup->boundingRect().height(), outputsGroup->boundingRect().height());

	qreal componentHeight =
	    verticalElementsSpacer +
	    name->boundingRect().height() +
	    verticalElementsSpacer +
	    maxVariablesHeight +
	    verticalElementsSpacer;

	//
	// Draw component border

	auto border = new QGraphicsRectItem(0 ,0, componentWidth, componentHeight);

	//
	// Add items to scene

	this->addItem(border);
	this->addItem(name);
	this->addItem(inputsGroup);
	this->addItem(outputsGroup);

	// Items position on scene:
	// Component top left corner @ (0; 0)

	border->setPos(0, 0);

	name->setPos( (componentWidth-name->boundingRect().width())/2, verticalElementsSpacer);

	qreal verticalVariablesNameOffset = name->boundingRect().bottom() + verticalElementsSpacer;

	qreal inoutsDeltaHeight = inputsGroup->boundingRect().height() - outputsGroup->boundingRect().height();
	qreal additionalInputsOffet  = (inoutsDeltaHeight > 0 ? 0 : -inoutsDeltaHeight/2);
	qreal additionalOutputsOffet = (inoutsDeltaHeight < 0 ? 0 : inoutsDeltaHeight/2);

	inputsGroup-> setPos(0,              verticalVariablesNameOffset + additionalInputsOffet);
	outputsGroup->setPos(componentWidth, verticalVariablesNameOffset + additionalOutputsOffet);

	//
	// Destroy groups as they mess with getItemsBoundingRect()

	this->destroyItemGroup(inputsGroup);
	this->destroyItemGroup(outputsGroup);
}
