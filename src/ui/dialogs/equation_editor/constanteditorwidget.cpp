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
#include "constanteditorwidget.h"

// Qt classes
#include <QVBoxLayout>
#include <QLabel>
#include <QDrag>

// StateS classes
#include "equation.h"
#include "valueeditor.h"
#include "equationpartmimedata.h"
#include "equationeditorwidget.h"


ConstantEditorWidget::ConstantEditorWidget(LogicValue initialValue, uint rankInParentOperands, bool isTemplate, QWidget* parent) :
	EquationPartEditorWidget(rankInParentOperands, isTemplate, parent)
{
	this->currentValue = initialValue;

	this->build();
}

void ConstantEditorWidget::beginEdit()
{
	if (this->getIsEditable() == false) return;

	if (this->isEditing == true) return;


	if (this->parentEquationEditor != nullptr)
	{
		this->parentEquationEditor->closeOpenEditors();
	}

	this->clear();
	this->isEditing = true;
	this->build();

	if (this->valueEditor != nullptr)
	{
		this->valueEditor->setFocus();
	}
}

bool ConstantEditorWidget::validEdit()
{
	if (this->isEditing == false) return false;

	if (this->valueEditor == nullptr) return false;


	this->tempConstant = this->valueEditor->getBitVectorValue();
	this->tempValueNature = OperandSource_t::constant;
	this->replaceByTempValue();
	// Do NOT do anything after, as this ceases to exist

	return true;
}

bool ConstantEditorWidget::cancelEdit()
{
	if (this->isEditing == false) return false;


	this->clear();
	this->isEditing = false;
	this->build();

	return true;
}

uint ConstantEditorWidget::getAllowedMenuActions() const
{
	return (uint)ContextAction_t::Remove |
	       (uint)ContextAction_t::Edit   |
	       (uint)ContextAction_t::Invert;
}

uint ConstantEditorWidget::getAllowedDropActions() const
{
	return (uint)DropAction_t::ReplaceExisting |
	       (uint)DropAction_t::ExistingAsOperand;
}

QString ConstantEditorWidget::getText() const
{
	if (this->currentValue.isNull() == true) return QString();


	return this->currentValue.toString();
}

QString ConstantEditorWidget::getToolTipText() const
{
	return QString(tr("Constant:") + " " + this->getText());
}

bool ConstantEditorWidget::getReplaceWithoutAsking() const
{
	return true;
}

bool ConstantEditorWidget::getIsErroneous() const
{
	return this->currentValue.isNull();
}

bool ConstantEditorWidget::getIsEditable() const
{
	return !this->isTemplate;
}

QDrag* ConstantEditorWidget::buildDrag()
{
	auto drag = new QDrag(this);

	uint availableAction = (uint)DropAction_t::ReplaceExisting;

	auto mimeData = new EquationPartMimeData(tr("Custom constant"), availableAction, this->currentValue);
	drag->setMimeData(mimeData);
	drag->setPixmap(this->grab());

	return drag;
}

void ConstantEditorWidget::processSpecificMenuAction(ContextAction_t action)
{
	switch (action)
	{
	case ContextAction_t::Invert:
		this->tempValueNature = OperandSource_t::equation;
		this->tempEquation = make_shared<Equation>(OperatorType_t::notOp);
		this->tempEquation->setOperand(0, this->currentValue);
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
	case ContextAction_t::IncreaseOperandCount:
	case ContextAction_t::DecreaseOperandCount:
	case ContextAction_t::ExtractSwitchRange:
	case ContextAction_t::ExtractSwitchSingle:
	case ContextAction_t::AddExtractor:
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

void ConstantEditorWidget::processSpecificDropAction(DropAction_t action)
{
	switch (action)
	{
	case DropAction_t::ExistingAsOperand:
		if (this->tempEquation == nullptr) return;


		this->tempEquation->setOperand(0, this->currentValue);
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
	case DropAction_t::RemoveInverter:
		//  This action is not supported by this class
		break;
	case DropAction_t::ReplaceExisting:
	case DropAction_t::Cancel:
		// Common actions: processed by parent class
		break;
	}
}

void ConstantEditorWidget::valueChangedEventHandler()
{
	this->validEdit();
}

void ConstantEditorWidget::cancelEditEventHandler()
{
	this->cancelEdit();
}

void ConstantEditorWidget::clear()
{
	if (this->valueEditor != nullptr)
	{
		this->valueEditor->deleteLater();
		this->valueEditor = nullptr;
	}
	if (this->valueText != nullptr)
	{
		this->valueText->deleteLater();
		this->valueText = nullptr;
	}

	delete this->layout();
}

void ConstantEditorWidget::build()
{
	if (this->isTemplate == false)
	{
		if (this->currentValue.isNull() == true) return;


		auto layout = new QVBoxLayout(this);

		if (this->isEditing == false)
		{
			this->valueText = new QLabel(this->currentValue.toString());

			layout->addWidget(this->valueText);
		}
		else // (this->isEditing == true)
		{
			this->valueEditor = new ValueEditor();
			this->valueEditor->setFocusOnShow(true);
			this->valueEditor->setBitVectorValue(this->currentValue);

			connect(this->valueEditor, &ValueEditor::valueChangedEvent, this, &ConstantEditorWidget::valueChangedEventHandler);
			connect(this->valueEditor, &ValueEditor::cancelEditEvent,   this, &ConstantEditorWidget::cancelEditEventHandler);

			layout->addWidget(this->valueEditor);
		}
	}
	else // (this->isTemplate == true)
	{
		auto layout = new QVBoxLayout(this);

		this->valueText = new QLabel(tr("Custom constant"));
		this->valueText->setAlignment(Qt::AlignCenter);

		layout->addWidget(this->valueText);
	}
}
