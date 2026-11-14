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
#include "actionconditioneditor.h"

// Qt
#include <QLabel>

// StateS
#include "equation.h"
#include "equationeditordialog.h"


ActionConditionEditor::ActionConditionEditor(QWidget* parent) :
	CellWithEditAndClearButtonsEditor{parent}
{
	this->conditionText = new QLabel();
	this->setMainWidget(this->conditionText);
}

void ActionConditionEditor::setCondition(std::shared_ptr<const Equation> condition)
{
	if (condition != nullptr)
	{
		this->condition = condition->clone();
	}
	else
	{
		this->condition = nullptr;
	}

	this->updateDisplay();
}

std::shared_ptr<Equation> ActionConditionEditor::getCondition() const
{
	return this->condition;
}

void ActionConditionEditor::editButtonClickedEventHandler()
{
	this->equationEditorDialog = new EquationEditorDialog(this->condition, this);
	connect(this->equationEditorDialog, &EquationEditorDialog::finished, this, &ActionConditionEditor::equationEditorDialogClosedEventHandler);

	this->equationEditorDialog->open();
}

void ActionConditionEditor::clearButtonClickedEventHandler()
{
	this->condition.reset();

	this->updateDisplay();
	emit this->contentChangedEvent();
}

void ActionConditionEditor::equationEditorDialogClosedEventHandler(int result)
{
	if (this->equationEditorDialog == nullptr) return;


	if (result == QDialog::DialogCode::Accepted)
	{
		this->condition = this->equationEditorDialog->getResultEquation();

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

bool ActionConditionEditor::shouldClearButtonBeVisibile()
{
	if (this->condition != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ActionConditionEditor::updateDisplay()
{
	if (this->condition != nullptr)
	{
		auto equationRawText = this->condition->getText();

		// Set tooltip and format text

		this->setToolTip(QString{});

		auto equationType = this->condition->getType();
		if (equationType == MachineValue::Type_t::nullType)
		{
			// The equation is invalid
			equationRawText = "<span style=\"color:red;\">" + equationRawText + "</span>";

			QString toolTipText = tr("Warning:") + " " + tr("The current condition is not valid.");
			this->setToolTip(toolTipText);
		}
		else if (equationType == MachineValue::Type_t::boolean)
		{
			// Ok
		}
		else if ( (equationType == MachineValue::Type_t::bitVector) && (this->condition->getInitialValue().getBitVectorValue().getSize() == 1) )
		{
			// Ok
		}
		else
		{
			// The equation is valid but neither
			// a boolean nor a bit vector of size 1
			equationRawText = "<span style=\"color:red;\">" + equationRawText + "</span>";

			QString toolTipText = tr("Warning:") + " " + tr("The condition can't be interpreted as a boolean.");
			this->setToolTip(toolTipText);
		}

		// Set content

		this->conditionText->setText(equationRawText);
	}
	else // (this->condition == nullptr)
	{
		this->conditionText->clear();
	}

	this->updateClearButtonVisibility();
}
