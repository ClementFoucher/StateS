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
#include "actioneditor.h"

// Qt classes
#include <QLayout>
#include <QPushButton>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "contextmenu.h"
#include "hintwidget.h"
#include "actiontableview.h"


bool ActionEditor::hintCollapsed = false;


ActionEditor::ActionEditor(componentId_t actuatorId, QWidget* parent) :
	QWidget(parent)
{
	//
	// Table
	this->actionTable = new ActionTableView(actuatorId, this);
	this->actionTable->initialize();

	connect(this->actionTable, &ActionTableView::selectionFlagsChangedEvent, this, &ActionEditor::updateButtonsEnableState);

	//
	// Buttons
	this->buttonAddAction    = new QPushButton(tr("Add action"),    this);
	this->buttonRemoveAction = new QPushButton(tr("Remove action"), this);
	this->buttonMoveUp       = new QPushButton("↥",                 this);
	this->buttonMoveDown     = new QPushButton("↧",                 this);

	this->buttonRemoveAction->setEnabled(false);
	this->buttonMoveUp      ->setEnabled(false);
	this->buttonMoveDown    ->setEnabled(false);

	connect(this->buttonAddAction,    &QPushButton::clicked, this, &ActionEditor::buttonAddPushedEventHandler);
	connect(this->buttonRemoveAction, &QPushButton::clicked, this, &ActionEditor::buttonRemovePushedEventHandler);
	connect(this->buttonMoveUp,       &QPushButton::clicked, this, &ActionEditor::buttonMoveUpPushedEventHandler);
	connect(this->buttonMoveDown,     &QPushButton::clicked, this, &ActionEditor::buttonMoveDownPushedEventHandler);

	//
	// Hint
	QString hintTitle = tr("Hint:") + " " + tr("Editing actions");

	QString hint;
	hint += "<br />";
	hint += tr("If action value is editable,") + " " + tr("double-click on it") + " " + tr("to edit it.");
	hint += "<br />";
	hint += tr("Right-click") + " " + tr("on an action") + " " + tr("to display more edit options.");
	hint += "<br />";

	this->hintDisplay = new HintWidget(hintTitle, hint, this);
	if (ActionEditor::hintCollapsed == true)
	{
		this->hintDisplay->setCollapsed(true);
	}

	//
	// Build complete rendreding
	auto buttonLayout = new QGridLayout();
	buttonLayout->addWidget(this->buttonAddAction,    0, 0, 1, 2);
	buttonLayout->addWidget(this->buttonRemoveAction, 0, 2, 1, 2);
	buttonLayout->addWidget(this->buttonMoveUp,       0, 4, 1, 1);
	buttonLayout->addWidget(this->buttonMoveDown,     0, 5, 1, 1);

	auto layout = new QVBoxLayout(this);
	layout->addWidget(this->actionTable);
	layout->addLayout(buttonLayout);
	layout->addWidget(this->hintDisplay);
}

ActionEditor::~ActionEditor()
{
	ActionEditor::hintCollapsed = this->hintDisplay->getCollapsed();
}

void ActionEditor::buttonAddPushedEventHandler()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	ContextMenu* menu;

	auto writtableVariablesIds = machine->getWrittableVariablesIds();
	if (writtableVariablesIds.count() != 0)
	{
		menu = new ContextMenu();
		menu->setListStyle();

		menu->addTitle(tr("Availables variables:"));
		for (auto& variableId : writtableVariablesIds)
		{
			auto variable = machine->getVariable(variableId);
			if (variable == nullptr) continue;


			menu->addAction(variable->getName());
		}

		connect(menu, &QMenu::triggered, this, &ActionEditor::processAddActionMenuEventHandler);
	}
	else
	{
		auto textList = QStringList();
		textList.append(tr("No compatible variable!"));
		textList.append(tr("First declare the machine variables in Machine tab."));
		menu = ContextMenu::createErrorMenu(textList);
	}

	menu->popup(this->buttonAddAction->mapToGlobal(QPoint(0, -menu->sizeHint().height())));
}

void ActionEditor::buttonRemovePushedEventHandler()
{
	this->actionTable->deleteSelectedRows();
}

void ActionEditor::buttonMoveUpPushedEventHandler()
{
	this->actionTable->raiseSelectedRows();
}

void ActionEditor::buttonMoveDownPushedEventHandler()
{
	this->actionTable->lowerSelectedRows();
}

void ActionEditor::processAddActionMenuEventHandler(QAction* action)
{
	this->actionTable->addAction(action->text());
}

void ActionEditor::updateButtonsEnableState()
{
	this->buttonRemoveAction->setEnabled(this->actionTable->getSelectionCanBeDeleted());
	this->buttonMoveUp      ->setEnabled(this->actionTable->getSelectionCanBeRaised());
	this->buttonMoveDown    ->setEnabled(this->actionTable->getSelectionCanBeLowered());
}
