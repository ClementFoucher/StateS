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
#include "equationeditorcell.h"

// Qt
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

// StateS
#include "equation.h"
#include "equationeditordialog.h"


EquationEditorCell::EquationEditorCell(QWidget* parent) :
	QWidget{parent}
{
	this->equationText = new QLabel();
	this->buttonEditEquation = new QPushButton("📝");
	this->buttonEditEquation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	this->mainLayout = new QHBoxLayout(this);

	// Set main spacing to 0, but retain initial value
	auto spacing = this->mainLayout->spacing();
	this->mainLayout->setSpacing(0);

	// Only keep the left margin
	auto contentMargins = this->mainLayout->contentsMargins();
	contentMargins.setBottom(0);
	contentMargins.setTop(0);
	contentMargins.setRight(0);
	this->mainLayout->setContentsMargins(contentMargins);

	this->mainLayout->addWidget(this->equationText,       0, Qt::AlignLeft);
	this->mainLayout->addSpacing(spacing); // Restore minimal spacing between text and buttons
	this->mainLayout->addStretch();        // Add additional strech in case the cell is wider
	this->mainLayout->addWidget(this->buttonEditEquation, 0, Qt::AlignRight);

	connect(this->buttonEditEquation, &QPushButton::pressed, this, &EquationEditorCell::editEquationButtonPressedEventHandler);
}

void EquationEditorCell::setEquation(std::shared_ptr<const Equation> equation)
{
	if (equation == nullptr) return;


	this->equation = equation->clone();

	this->updateDisplay();
}

std::shared_ptr<Equation> EquationEditorCell::getEquation() const
{
	return this->equation;
}

void EquationEditorCell::setIsConditionEditor(bool isConditionEditor)
{
	this->isConditionEditor = isConditionEditor;
}

void EquationEditorCell::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	int height = this->height();

	this->buttonEditEquation->setFixedWidth(height);
	if (this->buttonClearEquation != nullptr)
	{
		this->buttonClearEquation->setFixedWidth(height);
	}
}

void EquationEditorCell::editEquationButtonPressedEventHandler()
{
	this->equationEditorDialog = new EquationEditorDialog(this->equation, this);
	connect(this->equationEditorDialog, &EquationEditorDialog::finished, this, &EquationEditorCell::equationEditorDialogClosedEventHandler);

	this->equationEditorDialog->open();
}

void EquationEditorCell::clearEquationButtonPressedEventHandler()
{
	this->equation.reset();

	this->updateDisplay();
	emit this->equationChangedEvent(this);
}

void EquationEditorCell::equationEditorDialogClosedEventHandler(int result)
{
	if (this->equationEditorDialog == nullptr) return;


	if (result == QDialog::DialogCode::Accepted)
	{
		this->equation = this->equationEditorDialog->getResultEquation();

		this->updateDisplay();
		emit this->equationChangedEvent(this);
	}

	delete this->equationEditorDialog;
	this->equationEditorDialog = nullptr;
}

void EquationEditorCell::updateDisplay()
{
	if (this->equation != nullptr)
	{
		auto equationRawText = this->equation->getText();
		auto equationType = this->equation->getType();

		this->setToolTip(QString{});

		if (equationType == MachineValue::Type_t::nullType)
		{
			// The equation is invalid
			equationRawText =  "<span style=\"color:red;\">" + equationRawText + "</span>";

			QString toolTipText = tr("Warning:") + " " + tr("The current condition is not valid.");
			this->setToolTip(toolTipText);
		}
		else if (this->isConditionEditor == true)
		{
			if (this->equation->getType() == MachineValue::Type_t::boolean)
			{
				// Ok
			}
			else if ( (this->equation->getType() == MachineValue::Type_t::bitVector) && (this->equation->getInitialValue().getBitVectorValue().getSize() == 1) )
			{
				// Ok
			}
			else
			{
				// The equation is valid but neither
				// a boolean nor a bit vector of size 1
				equationRawText =  "<span style=\"color:red;\">" + equationRawText + "</span>";

				QString toolTipText = tr("Warning:") + " " + tr("The condition can't be interpreted as a boolean.");
				this->setToolTip(toolTipText);
			}
		}

		this->equationText->setText(equationRawText);

		if (this->buttonClearEquation == nullptr)
		{
			this->buttonClearEquation = new QPushButton("⌫");
			this->buttonClearEquation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			this->buttonClearEquation->setStyleSheet("QPushButton {color: red;}");
			this->mainLayout->addWidget(this->buttonClearEquation, 0, Qt::AlignRight);

			connect(this->buttonClearEquation, &QPushButton::pressed, this, &EquationEditorCell::clearEquationButtonPressedEventHandler);
		}
		else // (this->buttonClearEquation != nullptr)
		{
			this->buttonClearEquation->show();
		}
	}
	else // (this->equation == nullptr)
	{
		this->equationText->clear();

		if (this->buttonClearEquation != nullptr)
		{
			this->buttonClearEquation->hide();
		}
	}
}
