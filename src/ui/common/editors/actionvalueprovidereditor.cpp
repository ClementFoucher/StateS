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
#include "actionvalueprovidereditor.h"

// Qt
#include <QLabel>

// StateS
#include "equation.h"
#include "operand.h"
#include "valueeditor.h"
#include "equationeditordialog.h"


ActionValueProviderEditor::ActionValueProviderEditor(QWidget* parent) :
	CellWithEditAndClearButtonsEditor{parent}
{
	this->setTextAsMainWidget();
}

void ActionValueProviderEditor::setValueProvider(std::shared_ptr<const Equation> valueProvider)
{
	if (valueProvider != nullptr)
	{
		this->valueProvider = valueProvider->clone();
	}
	else
	{
		this->valueProvider = nullptr;
	}

	this->updateDisplay();
}

std::shared_ptr<Equation> ActionValueProviderEditor::getValueProvider() const
{
	return this->valueProvider;
}

void ActionValueProviderEditor::setExpectedType(MachineValue::Type_t expectedType, uint bitVectorExpectedSize)
{
	this->expectedType          = expectedType;
	this->expectedBitVectorSize = bitVectorExpectedSize;

	this->updateDisplay();
}

void ActionValueProviderEditor::editButtonClickedEventHandler()
{
	this->equationEditorDialog = new EquationEditorDialog(this->valueProvider, this);
	connect(this->equationEditorDialog, &EquationEditorDialog::finished, this, &ActionValueProviderEditor::equationEditorDialogClosedEventHandler);

	this->equationEditorDialog->open();
}

void ActionValueProviderEditor::clearButtonClickedEventHandler()
{
	this->valueProvider.reset();

	this->updateDisplay();

	emit this->contentChangedEvent();
}

void ActionValueProviderEditor::equationEditorDialogClosedEventHandler(int result)
{
	if (this->equationEditorDialog == nullptr) return;


	if (result == QDialog::DialogCode::Accepted)
	{
		this->valueProvider = this->equationEditorDialog->getResultEquation();

		this->updateDisplay();

		delete this->equationEditorDialog;
		this->equationEditorDialog = nullptr;

		emit this->contentChangedEvent();
	}
	else
	{
		delete this->equationEditorDialog;
		this->equationEditorDialog = nullptr;
	}
}

void ActionValueProviderEditor::constantEditorValueChangedEventHandler()
{
	auto newConstant = this->constantEditor->getMachineValue();

	// Constant editor is only active when value provider is already a constant
	this->valueProvider->setOperand(0, newConstant);

	emit this->contentChangedEvent();
}

bool ActionValueProviderEditor::shouldClearButtonBeVisibile()
{
	if (this->valueProvider == nullptr)
	{
		return false;
	}
	else
	{
		if (this->isCurrentValueProviderConstant() == true)
		{
			return false;
		}
		else // (this->isCurrentValueProviderConstant() == false)
		{
			return true;
		}
	}
}

void ActionValueProviderEditor::updateDisplay()
{
	this->setToolTip(QString{});

	// Display warnings
	bool valueProviderIsInvalid = false;
	if (this->valueProvider == nullptr)
	{
		valueProviderIsInvalid = true;

		QString toolTipText = tr("Warning:") + " " + tr("The current action is empty.");
		this->setToolTip(toolTipText);
	}
	else if (this->isCurrentValueProviderConstant() == false)
	{
		auto equationType = this->valueProvider->getType();
		if (equationType == MachineValue::Type_t::nullType)
		{
			valueProviderIsInvalid = true;

			QString toolTipText = tr("Warning:") + " " + tr("The current equation is not valid.");
			this->setToolTip(toolTipText);
		}
		else if (equationType != this->expectedType)
		{
			valueProviderIsInvalid = true;

			QString toolTipText = tr("Warning:") + " " + tr("The current equation result type does not match the expected action type.");
			this->setToolTip(toolTipText);
		}
	}

	// Update main widget
	if (this->valueProvider != nullptr)
	{
		if (this->isCurrentValueProviderConstant() == true)
		{
			this->setConstantEditorAsMainWidget();

			this->constantEditor->setMachineValue(this->valueProvider->getInitialValue());
			if (this->expectedType == MachineValue::Type_t::bitVector)
			{
				this->constantEditor->setBitVectorSize(this->expectedBitVectorSize);
			}
		}
		else // (this->isCurrentValueProviderConstant() == false)
		{
			this->setTextAsMainWidget();

			auto equationRawText = this->valueProvider->getText();
			if (valueProviderIsInvalid == true)
			{
				equationRawText = "<span style=\"color:red;\">" + equationRawText + "</span>";
			}

			this->valueProviderText->setText(equationRawText);
		}
	}
	else // (this->valueProvider == nullptr)
	{
		this->setTextAsMainWidget();
		this->valueProviderText->setText("(" + tr("empty") + ")");
	}

	this->updateClearButtonVisibility();
}

void ActionValueProviderEditor::setTextAsMainWidget()
{
	if (this->valueProviderText != nullptr) return;


	this->valueProviderText = new QLabel();

	this->setMainWidget(this->valueProviderText);

	delete this->constantEditor;
	this->constantEditor = nullptr;
}

void ActionValueProviderEditor::setConstantEditorAsMainWidget()
{
	if (this->constantEditor != nullptr) return;


	this->constantEditor  = new ValueEditor();
	connect(this->constantEditor, &ValueEditor::valueChangedEvent, this, &ActionValueProviderEditor::constantEditorValueChangedEventHandler);

	this->setMainWidget(this->constantEditor);

	delete this->valueProviderText;
	this->valueProviderText = nullptr;
}

bool ActionValueProviderEditor::isCurrentValueProviderConstant() const
{
	if (this->expectedType == MachineValue::Type_t::nullType) return false;

	if (this->valueProvider == nullptr) return false;

	if (this->valueProvider->getOperator() != Equation::Operator_t::identity) return false;

	auto operand = this->valueProvider->getOperand(0);
	if (operand == nullptr) return false;

	if (operand->getSource() != Operand::Source_t::constant) return false;


	auto currentEquationValue = this->valueProvider->getInitialValue();
	auto currentEquationType  = currentEquationValue.getType();

	if (currentEquationType != this->expectedType)
	{
		return false;
	}
	else // (currentEquationType == this->expectedType)
	{
		if (currentEquationType != MachineValue::Type_t::bitVector)
		{
			return true;
		}
		else // (currentEquationType == MachineValue::Type_t::bitVector)
		{
			if (this->expectedBitVectorSize == 0)
			{
				return true;
			}
			else // (this->expectedBitVectorSize != 0)
			{
				return (currentEquationValue.getBitVectorValue().getSize() == this->expectedBitVectorSize);
			}
		}
	}

	return false;
}
