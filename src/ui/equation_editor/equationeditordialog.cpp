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
#include "equationeditordialog.h"

// Qt classes
#include <QLabel>
#include <QIcon>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QKeyEvent>
#include <QScrollBar>

// StateS classes
#include "equation.h"
#include "equationeditorwidget.h"
#include "templateequationpartswidget.h"


EquationEditorDialog::EquationEditorDialog(shared_ptr<const Equation> initialEquation, QWidget* parent) :
	StatesDialog(parent)
{
	//
	// Window properties
	this->setWindowTitle(tr("Equation editor"));

	//
	// Templates

	// Title text
	auto templatesTitle = new QLabel("<i>" + tr("Drag and drop equation components from here") + "…</i>");

	// Widget
	auto templatesWidget = new TemplateEquationPartsWidget();

	// Scroll area
	auto templatesScrollArea = new QScrollArea();
	templatesScrollArea->setWidgetResizable(true);
	templatesScrollArea->setWidget(templatesWidget);

	// Ensure width is always sufficient to avoid showing an horizontal scroll bar
	auto templatesScrollAreaWidth = templatesWidget->width();
	templatesScrollAreaWidth += templatesScrollArea->verticalScrollBar()->sizeHint().width();
	templatesScrollAreaWidth += 2*templatesScrollArea->frameWidth();
	templatesScrollArea->setMinimumWidth(templatesScrollAreaWidth);

	//
	// Equation

	// Title text
	QLabel* equationTitle = new QLabel("<i>… " + tr("to here.") + "</i>");

	// Widget
	if (initialEquation != nullptr)
	{
		this->equationDisplay = new EquationEditorWidget(initialEquation->clone(), false);
	}
	else
	{
		this->equationDisplay = new EquationEditorWidget(shared_ptr<Equation>(nullptr), false);
	}

	// Bottom text
	QLabel* equationInfo= new QLabel(tr("You can also use right-click on equation members to edit"));

	//
	// Buttons
	QHBoxLayout* buttonsLayout = new QHBoxLayout();

	QPushButton* buttonOK = new QPushButton(tr("OK"));
	buttonsLayout->addWidget(buttonOK);
	connect(buttonOK, &QAbstractButton::clicked, this, &EquationEditorDialog::accept);

	QPushButton* buttonCancel = new QPushButton(tr("Cancel"));
	buttonsLayout->addWidget(buttonCancel);
	connect(buttonCancel, &QAbstractButton::clicked, this, &EquationEditorDialog::reject);

	//
	// Build complete rendering
	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(templatesTitle, 0, Qt::AlignHCenter);
	mainLayout->addWidget(templatesScrollArea);
	mainLayout->addWidget(equationTitle, 0, Qt::AlignHCenter);
	mainLayout->addWidget(this->equationDisplay, 0, Qt::AlignHCenter);
	mainLayout->addWidget(equationInfo, 0, Qt::AlignHCenter);
	mainLayout->addLayout(buttonsLayout);
}

shared_ptr<Equation> EquationEditorDialog::getResultEquation() const
{
	return this->equationDisplay->getLogicEquation();
}

void EquationEditorDialog::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key_Return)
	{
		if (this->equationDisplay->validEdit() == true)
		{
			transmitEvent = false;
		}
	}
	else if (event->key() == Qt::Key_Escape)
	{
		if (this->equationDisplay->cancelEdit() == true)
		{
			transmitEvent = false;
		}
	}

	if (transmitEvent)
	{
		QDialog::keyPressEvent(event);
	}
}

void EquationEditorDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->equationDisplay->validEdit();
	}

	QDialog::mousePressEvent(event);
}
