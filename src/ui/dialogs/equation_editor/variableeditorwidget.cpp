/*
 * Copyright © 2025 Clément Foucher
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
#include "variableeditorwidget.h"

// Qt classes
#include <QLabel>
#include <QHBoxLayout>
#include <QDrag>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "equation.h"
#include "equationpartmimedata.h"


VariableEditorWidget::VariableEditorWidget(componentId_t variableId, uint rankInParentOperands, bool isTemplate, QWidget* parent) :
	EquationPartEditorWidget(rankInParentOperands, isTemplate, parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	this->variableId = variableId;

	// Text
	auto layout = new QHBoxLayout(this);
	auto text = new QLabel(variable->getName());
	text->setAlignment(Qt::AlignCenter);
	layout->addWidget(text);
}

uint VariableEditorWidget::getAllowedMenuActions() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return 0;


	uint allowedActions = (uint)ContextAction_t::Remove |
	                      (uint)ContextAction_t::Invert;

	if (variable->getSize() > 1)
	{
		allowedActions |= (uint)ContextAction_t::AddExtractor;
	}

	return allowedActions;
}

uint VariableEditorWidget::getAllowedDropActions() const
{
	return (uint)DropAction_t::ReplaceExisting |
	       (uint)DropAction_t::ExistingAsOperand;
}

QString VariableEditorWidget::getText() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QString();

	auto variable = machine->getVariable(this->variableId);
	if (variable == nullptr) return QString();


	return variable->getName();
}

QString VariableEditorWidget::getToolTipText() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QString();

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return QString();


	auto toolTipText = tr("Variable") + " <b>" + variable->getName() + "</b> " + tr("has type") + " ";
	auto value = variable->getInitialValue();
	uint size = value.getSize();
	if (size > 1)
	{
		toolTipText += tr("bit vector") + " " + tr("of") + " " + QString::number(size) + " " + tr("bits");
	}
	else
	{
		toolTipText += tr("bit");
	}

	return toolTipText;
}

bool VariableEditorWidget::getReplaceWithoutAsking() const
{
	return true;
}

bool VariableEditorWidget::getIsErroneous() const
{
	if (this->variableId == nullId) return true;


	return false;
}

QDrag* VariableEditorWidget::buildDrag()
{
	auto drag = new QDrag(this);

	uint availableAction = (uint)DropAction_t::ReplaceExisting;

	auto mimeData = new EquationPartMimeData(this->getText(), availableAction, this->variableId);
	drag->setMimeData(mimeData);
	drag->setPixmap(this->grab());

	return drag;
}

void VariableEditorWidget::processSpecificMenuAction(ContextAction_t action)
{
	switch (action)
	{
	case ContextAction_t::Invert:
		this->tempValueNature = OperandSource_t::equation;
		this->tempEquation = make_shared<Equation>(OperatorType_t::notOp);
		this->tempEquation->setOperand(0, this->variableId);
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
	case ContextAction_t::AddExtractor:
		this->tempValueNature = OperandSource_t::equation;
		this->tempEquation = make_shared<Equation>(OperatorType_t::extractOp);
		this->tempEquation->setOperand(0, this->variableId);
		this->tempEquation->setRange(0);
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
	case ContextAction_t::IncreaseOperandCount:
	case ContextAction_t::DecreaseOperandCount:
	case ContextAction_t::ExtractSwitchRange:
	case ContextAction_t::ExtractSwitchSingle:
	case ContextAction_t::RemoveInverter:
		// These actions are not supported by this class
		break;
	case ContextAction_t::Remove:
	case ContextAction_t::Cancel:
	case ContextAction_t::Edit:
		// Common actions: processed by parent class
		break;
	}
}

void VariableEditorWidget::processSpecificDropAction(DropAction_t action)
{
	switch (action)
	{
	case DropAction_t::ExistingAsOperand:
		if (this->tempEquation == nullptr) return;


		this->tempEquation->setOperand(0, this->variableId);
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
	case DropAction_t::RemoveInverter:
		// This action is not supported by this class
		break;
	case DropAction_t::ReplaceExisting:
	case DropAction_t::Cancel:
		// Common actions: processed by parent class
		break;
	}
}
