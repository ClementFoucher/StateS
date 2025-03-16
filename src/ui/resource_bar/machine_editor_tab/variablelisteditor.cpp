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
#include "variablelisteditor.h"

// Qt classes
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>

// StateS classes
#include "statestypes.h"
#include "variabletableview.h"


VariableListEditor::VariableListEditor(VariableNature_t editorNature, QWidget* parent) :
	QWidget(parent)
{
	this->editorNature = editorNature;

	//
	// Table
	this->tableView = new VariableTableView(this->editorNature);

	// To enable/disable buttons when a variable is selected
	connect(this->tableView, &VariableTableView::variablesSelectionChangedEvent, this, &VariableListEditor::updateButtonsEnableState);

	//
	// Buttons
	auto buttonLayout = new QGridLayout();

	this->buttonAdd    = new QPushButton(tr("Add"));
	this->buttonRemove = new QPushButton(tr("Remove"));
	this->buttonUp     = new QPushButton("↥");
	this->buttonDown   = new QPushButton("↧");

	buttonLayout->addWidget(this->buttonAdd,    0, 0,  1, 20);
	buttonLayout->addWidget(this->buttonRemove, 0, 20, 1, 20);
	buttonLayout->addWidget(this->buttonUp,     0, 40, 1, 5);
	buttonLayout->addWidget(this->buttonDown,   0, 45, 1, 5);

	this->buttonRemove->setEnabled(false);
	this->buttonUp->setEnabled(false);
	this->buttonDown->setEnabled(false);

	connect(this->buttonAdd,    &QPushButton::clicked, this, &VariableListEditor::buttonAddPressedEventHandler);
	connect(this->buttonRemove, &QPushButton::clicked, this, &VariableListEditor::buttonRemovePressedEventHandler);
	connect(this->buttonUp,     &QPushButton::clicked, this, &VariableListEditor::buttonUpPressedEventHandler);
	connect(this->buttonDown,   &QPushButton::clicked, this, &VariableListEditor::buttonDownPressedEventHandler);

	//
	// Build complete rendreding
	auto layout = new QVBoxLayout(this);

	layout->addWidget(this->tableView);
	layout->addLayout(buttonLayout);
}

void VariableListEditor::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Delete)
	{
		this->tableView->deleteSelectedVariables();
	}
	else
	{
		QWidget::keyPressEvent(event);
	}
}

void VariableListEditor::keyReleaseEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Delete)
	{
		// KeyPressEvent handled, so we have to handle keyReleaseEvent too.
	}
	else
	{
		QWidget::keyPressEvent(event);
	}
}

void VariableListEditor::updateButtonsEnableState()
{
	this->buttonRemove->setEnabled(this->tableView->getSelectedVariablesCanBeDeleted());
	this->buttonUp    ->setEnabled(this->tableView->getSelectedVariablesCanBeRaised());
	this->buttonDown  ->setEnabled(this->tableView->getSelectedVariablesCanBeLowered());
}

void VariableListEditor::buttonAddPressedEventHandler()
{
	this->tableView->addNewVariable();
}

void VariableListEditor::buttonRemovePressedEventHandler()
{
	this->tableView->deleteSelectedVariables();
}

void VariableListEditor::buttonUpPressedEventHandler()
{
	this->tableView->raiseSelectedVariables();
}

void VariableListEditor::buttonDownPressedEventHandler()
{
	this->tableView->lowerSelectedVariables();
}
