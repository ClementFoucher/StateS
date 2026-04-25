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
#include "variabletablescene.h"

// Qt classes
#include <QGraphicsTextItem>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"


void VariableTableScene::buildScene(bool displayInputs, bool displayOutputs, bool displayVariables, bool displayConstants)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	//
	// Remove previous build and reset positions

	this->clear();
	this->rightestHorizontalPos = 0;
	this->lowestVerticalPos     = 0;

	//
	// Prepare required variables

	qreal inputsTop    = 0;
	qreal outputsTop   = 0;
	qreal variablesTop = 0;
	qreal constantsTop = 0;

	qreal inputsSubHeader    = 0;
	qreal outputsSubHeader   = 0;
	qreal variablesSubHeader = 0;
	qreal constantsSubHeader = 0;

	qreal inputsBottom    = 0;
	qreal outputsBottom   = 0;
	qreal variablesBottom = 0;
	qreal constantsBottom = 0;

	qreal variableNameHorizontalPos  = 0;
	qreal memorizedHorizontalPos     = 0;
	qreal initialValueHorizontalPos  = 0;
	qreal defaultValueHorizontalPos  = 0;
	qreal constantValueHorizontalPos = 0;

	QGraphicsTextItem* inputsTitle    = nullptr;
	QGraphicsTextItem* outputsTitle   = nullptr;
	QGraphicsTextItem* variablesTitle = nullptr;
	QGraphicsTextItem* constantsTitle = nullptr;

	//
	// Build sections titles and variables names

	auto varNameFunc = [](shared_ptr<Variable> variable)
	{
		QString varText = variable->getName();
		if (variable->getSize() > 1)
		{
			varText += "[" + QString::number(variable->getSize()-1) + "..0]";
		}
		return varText;
	};

	if (displayInputs == true)
	{
		auto variables = machine->getInputVariablesIds();
		if (variables.empty() == false)
		{
			inputsTitle = this->buildSectionTitle(tr("Inputs"), 0);

			inputsTop       = this->lowestVerticalPos + this->vSpace;
			inputsSubHeader = this->buildColumn(tr("Name"), variables, variableNameHorizontalPos, inputsTop, varNameFunc);
			inputsBottom    = this->lowestVerticalPos;
		}
		else
		{
			displayInputs = false;
		}
	}

	if (displayOutputs == true)
	{
		auto variables = machine->getOutputVariablesIds();
		if (variables.empty() == false)
		{
			qreal outputTitleTop = (this->lowestVerticalPos == 0) ? 0 : this->lowestVerticalPos + this->vSpace;
			outputsTitle = this->buildSectionTitle(tr("Outputs"), outputTitleTop);

			outputsTop       = this->lowestVerticalPos + this->vSpace;
			outputsSubHeader = this->buildColumn(tr("Name"), variables, variableNameHorizontalPos, outputsTop, varNameFunc);
			outputsBottom    = this->lowestVerticalPos;
		}
		else
		{
			displayOutputs = false;
		}
	}

	if (displayVariables == true)
	{
		auto variables = machine->getInternalVariablesIds();
		if (variables.empty() == false)
		{
			qreal variablesTitleTop = (this->lowestVerticalPos == 0) ? 0 : this->lowestVerticalPos + this->vSpace;
			variablesTitle = this->buildSectionTitle(tr("Internal variables"), variablesTitleTop);

			variablesTop       = this->lowestVerticalPos + this->vSpace;
			variablesSubHeader = this->buildColumn(tr("Name"), variables, variableNameHorizontalPos, variablesTop, varNameFunc);
			variablesBottom    = this->lowestVerticalPos;
		}
		else
		{
			displayVariables = false;
		}
	}

	if (displayConstants == true)
	{
		auto variables = machine->getConstantsIds();
		if (variables.empty() == false)
		{
			qreal constantsTitleTop = (this->lowestVerticalPos == 0) ? 0 : this->lowestVerticalPos + this->vSpace;
			constantsTitle = this->buildSectionTitle(tr("Constants"), constantsTitleTop);

			constantsTop       = this->lowestVerticalPos + this->vSpace;
			constantsSubHeader = this->buildColumn(tr("Name"), variables, variableNameHorizontalPos, constantsTop, varNameFunc);
			constantsBottom    = this->lowestVerticalPos;
		}
		else
		{
			displayConstants = false;
		}
	}

	//
	// Build memorized column

	memorizedHorizontalPos = this->rightestHorizontalPos;

	auto varMemorizedFunc = [](shared_ptr<Variable> variable)
	{
		if (variable->getMemorized() == true)
		{
			return tr("yes");

		}
		else
		{
			return tr("no");
		}
	};

	if (displayOutputs == true)
	{
		this->drawVerticalLine(memorizedHorizontalPos, outputsTop, outputsBottom);
		this->buildColumn(tr("Memorized"), machine->getOutputVariablesIds(), memorizedHorizontalPos, outputsTop, varMemorizedFunc);
	}

	if (displayVariables == true)
	{
		this->drawVerticalLine(memorizedHorizontalPos, variablesTop, variablesBottom);
		this->buildColumn(tr("Memorized"), machine->getInternalVariablesIds(), memorizedHorizontalPos, variablesTop, varMemorizedFunc);
	}

	//
	// Determine values columns to display

	bool hasMemorizedVariables    = false;
	bool hasNonMemorizedVariables = false;

	QList<componentId_t> variablesIds;
	if (displayVariables)
	{
		variablesIds += machine->getInternalVariablesIds();
	}
	if (displayOutputs)
	{
		variablesIds += machine->getOutputVariablesIds();
	}

	for (auto variableId : as_const(variablesIds))
	{
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) continue;


		QGraphicsTextItem* currentRowText = nullptr;
		if (variable->getMemorized() == true)
		{
			hasMemorizedVariables = true;
		}
		else
		{
			hasNonMemorizedVariables = true;
		}

		if ( (hasMemorizedVariables == true) && (hasNonMemorizedVariables == true) )
		{
			break;
		}
	}

	//
	// Build values columns

	constantValueHorizontalPos = this->rightestHorizontalPos;

	if (hasMemorizedVariables == true)
	{
		initialValueHorizontalPos = this->rightestHorizontalPos;

		auto varInitialValueFunc = [](shared_ptr<Variable> variable)
		{
			if (variable->getMemorized() == true)
			{
				return variable->getInitialValue().toString();
			}
			else
			{
				return QString(" ");
			}
		};

		if (displayOutputs == true)
		{
			this->drawVerticalLine(initialValueHorizontalPos, outputsTop, outputsBottom);
			this->buildColumn(tr("Initial value"), machine->getOutputVariablesIds(), initialValueHorizontalPos, outputsTop, varInitialValueFunc);
		}

		if (displayVariables == true)
		{
			this->drawVerticalLine(initialValueHorizontalPos, variablesTop, variablesBottom);
			this->buildColumn(tr("Initial value"), machine->getInternalVariablesIds(), initialValueHorizontalPos, variablesTop, varInitialValueFunc);
		}
	}

	if (hasNonMemorizedVariables == true)
	{
		defaultValueHorizontalPos =this->rightestHorizontalPos;

		auto varDefaultValueFunc = [](shared_ptr<Variable> variable)
		{
			if (variable->getMemorized() == false)
			{
				return variable->getInitialValue().toString();
			}
			else
			{
				return QString(" ");
			}
		};

		if (displayOutputs == true)
		{
			this->drawVerticalLine(defaultValueHorizontalPos, outputsTop, outputsBottom);
			this->buildColumn(tr("Default value"), machine->getOutputVariablesIds(), defaultValueHorizontalPos, outputsTop, varDefaultValueFunc);
		}

		if (displayVariables == true)
		{
			this->drawVerticalLine(defaultValueHorizontalPos, variablesTop, variablesBottom);
			this->buildColumn(tr("Default value"), machine->getInternalVariablesIds(), defaultValueHorizontalPos, variablesTop, varDefaultValueFunc);
		}
	}

	if (displayConstants == true)
	{
		auto constantValueFunc = [](shared_ptr<Variable> variable)
		{
			return variable->getInitialValue().toString();
		};

		this->drawVerticalLine(constantValueHorizontalPos, constantsTop, constantsBottom);
		this->buildColumn(tr("Value"), machine->getConstantsIds(), constantValueHorizontalPos, constantsTop, constantValueFunc);
	}

	//
	// Add final polish

	if (displayInputs == true)
	{
		this->centerAndFrameTitle(inputsTitle);
		this->frameTable(inputsTop, inputsSubHeader, inputsBottom);
	}

	if (displayOutputs == true)
	{
		this->centerAndFrameTitle(outputsTitle);
		this->frameTable(outputsTop, outputsSubHeader, outputsBottom);
	}

	if (displayVariables == true)
	{
		this->centerAndFrameTitle(variablesTitle);
		this->frameTable(variablesTop, variablesSubHeader, variablesBottom);
	}

	if (displayConstants == true)
	{
		this->centerAndFrameTitle(constantsTitle);
		this->frameTable(constantsTop, constantsSubHeader, constantsBottom);
	}
}

QGraphicsTextItem* VariableTableScene::buildSectionTitle(const QString& titleText, qreal verticalPos)
{
	auto titleGraphicObject = this->buildGraphicsTextItem("<span style=\"font-weight: bold;\">" + titleText + "</span>");
	titleGraphicObject->setPos(0, verticalPos);
	this->addItem(titleGraphicObject);

	qreal titleLowestVerticalPos = verticalPos + titleGraphicObject->boundingRect().height();
	if (titleLowestVerticalPos > this->lowestVerticalPos)
	{
		this->lowestVerticalPos = titleLowestVerticalPos;
	}

	return titleGraphicObject;
}

qreal VariableTableScene::buildColumn(const QString& header, const QList<componentId_t> variablesIds, qreal horizontalPos, qreal verticalPos, function<QString (shared_ptr<Variable>)> textFunc)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;


	auto columnHeader = this->buildGraphicsTextItem(header);
	columnHeader->setPos(horizontalPos, verticalPos);
	this->addItem(columnHeader);

	qreal columnRightestHorizontalPos = horizontalPos + columnHeader->boundingRect().width();
	qreal columnLowestVerticalPos     = verticalPos   + columnHeader->boundingRect().height();

	qreal columnTitleLow = columnLowestVerticalPos;

	for (auto variableId : variablesIds)
	{
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) continue;


		auto currentRowText = this->buildGraphicsTextItem(textFunc(variable));
		currentRowText->setPos(horizontalPos, columnLowestVerticalPos);
		this->addItem(currentRowText);

		columnLowestVerticalPos += currentRowText->boundingRect().height();

		qreal currentRowRightestHorizontalPos = horizontalPos + currentRowText->boundingRect().width();
		if (currentRowRightestHorizontalPos > columnRightestHorizontalPos)
		{
			columnRightestHorizontalPos = currentRowRightestHorizontalPos;
		}
	}

	if (columnRightestHorizontalPos > this->rightestHorizontalPos)
	{
		this->rightestHorizontalPos = columnRightestHorizontalPos;
	}

	if (columnLowestVerticalPos > this->lowestVerticalPos)
	{
		this->lowestVerticalPos = columnLowestVerticalPos;
	}

	return columnTitleLow;
}

void VariableTableScene::drawVerticalLine(qreal x, qreal y1, qreal y2)
{
	auto verticalLineItem = new QGraphicsLineItem(x, y1, x, y2);
	this->addItem(verticalLineItem);
}

void VariableTableScene::centerAndFrameTitle(QGraphicsTextItem* title)
{
	if (title == nullptr) return;


	qreal left = this->rightestHorizontalPos/2 - title->boundingRect().width()/2;
	title->setPos(left, title->pos().y());

	auto titleFrame = new QGraphicsRectItem(0, title->pos().y(), this->rightestHorizontalPos, title->boundingRect().height());
	this->addItem(titleFrame);
}

void VariableTableScene::frameTable(qreal sectionTop, qreal sectionSubHeader, qreal sectionBottom)
{
	auto subHeaderLine = new QGraphicsLineItem(0, sectionSubHeader, this->rightestHorizontalPos, sectionSubHeader);
	this->addItem(subHeaderLine);

	auto tableFrame = new QGraphicsRectItem(0, sectionTop, this->rightestHorizontalPos, sectionBottom-sectionTop);
	this->addItem(tableFrame);
}
