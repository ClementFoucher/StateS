/*
 * Copyright © 2014-2023 Clément Foucher
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

// C++ classes
#include <algorithm>

// Qt classes
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QKeyEvent>
#include <QHeaderView>

// Debug
#include <QDebug>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "actiontypecombobox.h"
#include "contextmenu.h"
#include "StateS_signal.h"
#include "rangeeditordialog.h"
#include "collapsiblewidgetwithtitle.h"
#include "actiontablemodel.h"
#include "actiontabledelegate.h"
#include "actiononsignal.h"
#include "statesexception.h"
#include "exceptiontypes.h"


ActionEditor::ActionEditor(componentId_t actuatorId, QString title, QWidget* parent) :
    QWidget(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(actuatorId));
	if (actuator == nullptr) return;


	this->actuatorId = actuatorId;

	QGridLayout* layout = new QGridLayout(this);

	if (title.size() != 0)
	{
		QLabel* actionListTitle = new QLabel(title, this);
		actionListTitle->setAlignment(Qt::AlignCenter);
		actionListTitle->setWordWrap(true);
		layout->addWidget(actionListTitle, 0, 0, 1, 4);
	}

	this->actionTable = new QTableView(this);
	ActionTableModel* tableModel = new ActionTableModel(actuatorId, this->actionTable);
	this->actionTable->setModel(tableModel);
	this->actionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->actionTable->setItemDelegate(new ActionTableDelegate(actuatorId, this->actionTable));
	this->actionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	connect(this->actionTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);
	connect(tableModel, &QAbstractItemModel::layoutChanged, this, &ActionEditor::tableChangedEventHandler);
	layout->addWidget(this->actionTable, 1, 0, 1, 4);

	this->buttonMoveUp       = new QPushButton("↥",                 this);
	this->buttonMoveDown     = new QPushButton("↧",                 this);
	this->buttonAddAction    = new QPushButton(tr("Add action"),    this);
	this->buttonRemoveAction = new QPushButton(tr("Remove action"), this);

	connect(this->buttonMoveUp,       &QPushButton::clicked, this, &ActionEditor::moveSelectedActionsUp);
	connect(this->buttonMoveDown,     &QPushButton::clicked, this, &ActionEditor::moveSelectedActionsDown);
	connect(this->buttonAddAction,    &QPushButton::clicked, this, &ActionEditor::displayAddActionMenu);
	connect(this->buttonRemoveAction, &QPushButton::clicked, this, &ActionEditor::removeSelectedActions);

	layout->addWidget(this->buttonMoveUp,       3, 0, 1, 1);
	layout->addWidget(this->buttonMoveDown,     3, 1, 1, 1);
	layout->addWidget(this->buttonAddAction,    3, 2, 1, 1);
	layout->addWidget(this->buttonRemoveAction, 3, 3, 1, 1);


	this->hintDisplay = new CollapsibleWidgetWithTitle(this);
	QString hintTitle = tr("Hint:") + " " + tr("Editing actions");

	QString hint;
	hint += "<br />";
	hint += tr("Double-click") + " " + tr("on an affected value") + " " + tr("to edit it.");
	hint += "<br />";
	hint += tr("Right-click") + " " + tr("on a vector signal") + " " + tr("to display range options.");
	hint += "<br />";

	this->hintDisplay->setContent(hintTitle, hint, true);

	layout->addWidget(this->hintDisplay, 4, 0, 1, 4);

	connect(actuator.get(), &MachineActuatorComponent::actionListChangedEvent, this->actionTable, &QTableView::resizeColumnsToContents);

	this->fillFirstColumn();
	this->updateButtonsEnableState();
}

void ActionEditor::keyPressEvent(QKeyEvent* e)
{
	bool transmitEvent = true;

	if (e->key() == Qt::Key::Key_Delete)
	{
		this->removeSelectedActions();
		transmitEvent = false;
	}

	if (transmitEvent == true)
	{
		QWidget::keyPressEvent(e);
	}
}

void ActionEditor::keyReleaseEvent(QKeyEvent* e)
{
	bool transmitEvent = true;

	if (e->key() == Qt::Key::Key_Delete)
	{
		transmitEvent = false;
	}

	if (transmitEvent == true)
	{
		QWidget::keyReleaseEvent(e);
	}
}

void ActionEditor::contextMenuEvent(QContextMenuEvent* event)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	QPoint correctedPos = this->actionTable->mapFromParent(event->pos());
	correctedPos.setX(correctedPos.x() - this->actionTable->verticalHeader()->width());
	correctedPos.setY(correctedPos.y() - this->actionTable->horizontalHeader()->height());
	int actionRank = this->actionTable->rowAt(correctedPos.y());

	if (actionRank >= 0)
	{
		try
		{
			shared_ptr<ActionOnSignal> actionActedOn = actuator->getAction(actionRank); // Throws StatesException

			ContextMenu* menu = new ContextMenu();
			menu->addTitle(tr("Action on signal") + " <i>" + actionActedOn->getSignalActedOn()->getName() + "</i>");

			QVariant data;
			QAction* actionToAdd = nullptr;

			if (actionActedOn->isActionValueEditable())
			{
				actionToAdd = menu->addAction(tr("Edit value"));
				int dataValue = ((int)ContextAction::EditValue) | (actionRank << 8);
				data.setValue(dataValue);
				actionToAdd->setData(data);
			}

			if (actionRank != 0)
			{
				actionToAdd = menu->addAction(tr("Move up"));
				int dataValue = ((int)ContextAction::MoveUp) | (actionRank << 8);
				data.setValue(dataValue);
				actionToAdd->setData(data);
			}

			if (actionRank != this->actionTable->model()->rowCount()-1)
			{
				actionToAdd = menu->addAction(tr("Move down"));
				int dataValue = ((int)ContextAction::MoveDown) | (actionRank << 8);
				data.setValue(dataValue);
				actionToAdd->setData(data);
			}

			menu->addSeparator();

			if (actionActedOn->getSignalActedOn()->getSize() > 1)
			{
				actionToAdd = menu->addAction(tr("Affect whole signal"));
				actionToAdd->setCheckable(true);
				if (actionActedOn->getActionRangeL() == -1)
					actionToAdd->setChecked(true);
				int dataValue = ((int)ContextAction::AffectSwitchWhole) | (actionRank << 8);
				data.setValue(dataValue);
				actionToAdd->setData(data);

				actionToAdd = menu->addAction(tr("Affect signal single bit"));
				actionToAdd->setCheckable(true);
				if ( (actionActedOn->getActionRangeL() != -1) && (actionActedOn->getActionRangeR() == -1) )
					actionToAdd->setChecked(true);
				dataValue = ((int)ContextAction::AffectSwitchSingle) | (actionRank << 8);
				data.setValue(dataValue);
				actionToAdd->setData(data);

				actionToAdd = menu->addAction(tr("Affect signal range"));
				actionToAdd->setCheckable(true);
				if ( (actionActedOn->getActionRangeL() != -1) && (actionActedOn->getActionRangeR() != -1) )
					actionToAdd->setChecked(true);
				dataValue = ((int)ContextAction::AffectSwitchRange) | (actionRank << 8);
				data.setValue(dataValue);
				actionToAdd->setData(data);

				if ( (actionActedOn->getActionRangeL() != -1) || (actionActedOn->getActionRangeR() != -1) )
				{
					if (actionActedOn->getActionRangeR() == -1)
						actionToAdd = menu->addAction(tr("Edit affected bit"));
					else
						actionToAdd = menu->addAction(tr("Edit range"));

					int dataValue = ((int)ContextAction::AffectEditRange) | (actionRank << 8);
					data.setValue(dataValue);
					actionToAdd->setData(data);
				}

				menu->addSeparator();
			}

			actionToAdd = menu->addAction(tr("Delete action"));
			int dataValue = ((int)ContextAction::DeleteAction) | (actionRank << 8);
			data.setValue(dataValue);
			actionToAdd->setData(data);

			actionToAdd = menu->addAction(tr("Cancel"));
			dataValue = ((int)ContextAction::Cancel) | (actionRank << 8);
			data.setValue(dataValue);
			actionToAdd->setData(data);

			menu->popup(this->mapToGlobal(event->pos()));

			connect(menu, &QMenu::triggered, this, &ActionEditor::processContextMenuEventHandler);
		}
		catch (StatesException e)
		{
			if ( (e.getSourceClass() != "MachineActuatorComponent") || (e.getEnumValue() != MachineActuatorComponentError_t::out_of_range) )
				throw;
		}
	}
}

void ActionEditor::selectionChangedEventHandler(const QItemSelection&, const QItemSelection&)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	this->updateButtonsEnableState();

	// Store selection to restore it on model update
	this->latestSelection.clear();

	QItemSelectionModel* selectionModel = this->actionTable->selectionModel();

	QModelIndexList selectedRows = selectionModel->selectedRows();

	for (QModelIndex index : selectedRows)
	{
		this->latestSelection.append(actuator->getAction(index.row()));
	}
}

void ActionEditor::displayAddActionMenu() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	ContextMenu* menu;

	QList<shared_ptr<Signal>> availableActions = machine->getWrittableSignals();
	if (availableActions.count() != 0)
	{
		menu = new ContextMenu();
		menu->setListStyle();

		for (shared_ptr<Signal> var : availableActions)
		{
			menu->addAction(var->getName());
		}

		connect(menu, &QMenu::triggered, this, &ActionEditor::processAddActionMenuEventHandler);
	}
	else
	{
		menu = ContextMenu::createErrorMenu(tr("No compatible signal!"));
	}

	menu->popup(this->buttonAddAction->mapToGlobal(QPoint(this->buttonAddAction->width(), -menu->sizeHint().height())));
}

void ActionEditor::removeSelectedActions()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	QModelIndexList indexList = this->actionTable->selectionModel()->selectedRows();
	if (indexList.isEmpty() == false)
	{
		this->latestSelection.clear();
		// Sort the list backwards to avoid index shifting while removing actions
		sort(indexList.rbegin(), indexList.rend());

		for (QModelIndex index : indexList)
		{
			actuator->removeAction(index.row()); // Throws StatesException - Ignored: list generated from action list
		}
		machineManager->notifyMachineEdited();
	}
}

void ActionEditor::processAddActionMenuEventHandler(QAction* action)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	QString signalName = action->text();
	// Find signal from name
	for (shared_ptr<Signal> var : machine->getWrittableSignals())
	{
		if (var->getName() == signalName)
		{
			actuator->addAction(var);
			machineManager->notifyMachineEdited();
			break;
		}
	}
}

void ActionEditor::processContextMenuEventHandler(QAction* action)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	int dataValue   = action->data().toInt();
	uint actionRank = (dataValue&0xFFF0)>>8;

	shared_ptr<ActionOnSignal> actionOnSignal = actuator->getAction(actionRank); // Throws StatesException - Ignored: list generated from action list

	int oldRangeL = actionOnSignal->getActionRangeL();
	int oldRangeR = actionOnSignal->getActionRangeR();
	QModelIndex index;

	int newRangeL = -1;
	int newRangeR = -1;
	bool setRange = false;

	switch (dataValue&0xF)
	{
	case ContextAction::Cancel:
		break;
	case ContextAction::EditValue:
		index = this->actionTable->model()->index(actionRank, 2);

		this->actionTable->setCurrentIndex(index);
		this->actionTable->edit(index);

		break;
	case ContextAction::DeleteAction:
		actuator->removeAction(actionRank); // Throws StatesException - Ignored: list generated from action list
		machineManager->notifyMachineEdited();
		break;
	case ContextAction::AffectSwitchWhole:
		if ( (oldRangeL != -1) || (oldRangeR != -1) )
		{
			setRange = true;
		}
		break;
	case ContextAction::AffectSwitchSingle:
		if (oldRangeL == -1)
		{
			newRangeL = 0;
			setRange = true;
		}
		else if  (oldRangeR != -1)
		{
			newRangeL = oldRangeL;
			setRange = true;
		}
		break;
	case ContextAction::AffectSwitchRange:
		if (oldRangeL == -1)
		{
			newRangeL = 1;
			newRangeR = 0;
			setRange = true;
		}
		else if  (oldRangeR == -1)
		{
			if (oldRangeL != 0)
			{
				newRangeL = oldRangeL;
				newRangeR = 0;
				setRange = true;
			}
			else
			{
				newRangeL = 1;
				newRangeR = 0;
				setRange = true;
			}
		}
		break;
	case ContextAction::MoveDown:
		actuator->changeActionRank(actionRank, actionRank+1);
		machineManager->notifyMachineEdited();
		break;
	case ContextAction::MoveUp:
		actuator->changeActionRank(actionRank, actionRank-1);
		machineManager->notifyMachineEdited();
		break;
	case ContextAction::AffectEditRange:
		unique_ptr<RangeEditorDialog>rangeEditor = unique_ptr<RangeEditorDialog>(new RangeEditorDialog(actionOnSignal));
		rangeEditor->exec();

		if (rangeEditor->result() == QDialog::Accepted)
		{
			newRangeL = rangeEditor->getRangeL();
			newRangeR = rangeEditor->getRangeR();
			setRange = true;
		}
		break;
	}

	try
	{
		if (setRange == true)
		{
			actionOnSignal->setActionRange(newRangeL, newRangeR);
		}
	}
	catch (const StatesException& e)
	{
		if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignalError_t::illegal_range) )
		{
			qDebug() << "(ActionEditor:) Warning: Incorrect range was set. Range change ignored.";
		}
		else
			throw;
	}
}

void ActionEditor::moveSelectedActionsUp()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	this->sortSelectionList();

	QList<int> selectedActionsRanks;

	QModelIndexList rows = this->actionTable->selectionModel()->selectedRows();
	for (QModelIndex index : rows)
	{
		selectedActionsRanks.append(index.row());
	}

	for (int i = 0 ; i < rows.count() ; i++)
	{
		// Actually lower upper signals rather than raise signal itself

		// Check if previous row is selected
		int currentActionRank = selectedActionsRanks.at(i) ;
		if (currentActionRank != 0)
		{
			bool previousSelected = false;
			for (QModelIndex index : this->actionTable->selectionModel()->selectedRows())
			{
				if (index.row() == currentActionRank-1)
				{
					previousSelected = true;
					break;
				}
			}

			if ( previousSelected == false )
			{
				actuator->changeActionRank(currentActionRank-1, currentActionRank);
			}
		}
	}
	machineManager->notifyMachineEdited();
}

void ActionEditor::moveSelectedActionsDown()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	this->sortSelectionList();

	QList<int> selectedActionsRanks;

	QModelIndexList rows = this->actionTable->selectionModel()->selectedRows();
	for (QModelIndex index : rows)
	{
		selectedActionsRanks.push_front(index.row());
	}

	for (int i = 0 ; i < rows.count() ; i++)
	{
		// Actually raise lower signals rater than lowering signal itself

		// Check if next row is selected
		int currentActionRank = selectedActionsRanks.at(i) ;
		if (currentActionRank != this->actionTable->model()->rowCount()-1)
		{
			bool nextSelected = false;
			for (QModelIndex index : this->actionTable->selectionModel()->selectedRows())
			{
				if (index.row() == currentActionRank+1)
				{
					nextSelected = true;
					break;
				}
			}

			if ( nextSelected == false )
			{
				actuator->changeActionRank(currentActionRank+1, currentActionRank);
			}
		}
	}
	machineManager->notifyMachineEdited();
}

void ActionEditor::tableChangedEventHandler()
{
	this->fillFirstColumn();
	this->restoreSelection();
	this->updateButtonsEnableState();
}

void ActionEditor::fillFirstColumn()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	if (actuator->getActions().count() != 0)
	{
		// Fill column with drop-down list
		for(int i = 0 ; i < this->actionTable->model()->rowCount() ; i++)
		{
			this->actionTable->setIndexWidget(this->actionTable->model()->index(i, 0), new ActionTypeComboBox(actuator->getAllowedActionTypes(), actuator->getAction(i))); // Throws StatesException - Ignored: list generated from action list
		}
	}
	else
	{
		// Clear potential previous widgets if action count drops to 0
		// (special message displayed)
		this->actionTable->setIndexWidget(this->actionTable->model()->index(0, 0), nullptr);
	}
}

void ActionEditor::updateButtonsEnableState()
{
	this->buttonMoveUp      ->setEnabled(false);
	this->buttonMoveDown    ->setEnabled(false);
	this->buttonRemoveAction->setEnabled(false);

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	if (actuator->getActions().count() != 0)
	{
		QItemSelectionModel* selectionModel = this->actionTable->selectionModel();

		// Update remove button state
		if (selectionModel->selectedRows().count() != 0)
		{
			this->buttonRemoveAction->setEnabled(true);
		}

		// Up/down buttons : only enable when relevant
		QModelIndexList rows = selectionModel->selectedRows();

		if (rows.count() != 0)
		{
			// First sort selected rows
			QVector<int> sortedRows(rows.count());
			for (int i = 0 ; i < rows.count() ; i++)
			{
				sortedRows[i] = rows[i].row();
			}

			sort(sortedRows.begin(), sortedRows.end());

			int currentRow = sortedRows[0];
			bool areSuccesive = true;

			// Then check if one or multiple groups
			for (int i = 1 ; i < sortedRows.count() ; i++)
			{
				if (sortedRows[i] == currentRow+1)
					currentRow = sortedRows[i];
				else
				{
					areSuccesive = false;
					break;
				}
			}

			if (areSuccesive == false)
			{
				// Lacunar selection can always be moved up or down
				this->buttonMoveUp->setEnabled(true);
				this->buttonMoveDown->setEnabled(true);
			}
			else
			{
				// If single group, check if at top or at bottom
				if (sortedRows[0] != 0)
					this->buttonMoveUp->setEnabled(true);

				if (sortedRows.last() != this->actionTable->model()->rowCount()-1)
					this->buttonMoveDown->setEnabled(true);
			}
		}
	}
}

/**
 * @brief ActionEditor::sortSelectionList makes sure
 * the selection ordrer of the items in list is ascending.
 * This is useful for moving up/down actions process.
 */
void ActionEditor::sortSelectionList()
{
	QItemSelectionModel* selectionModel = this->actionTable->selectionModel();

	disconnect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);

	QItemSelection selectedItems = selectionModel->selection();

	selectionModel->clearSelection();

	if ( ! selectedItems.isEmpty() )
	{
		for (int i = 0 ; i < this->actionTable->model()->rowCount() ; i++)
		{
			for (QModelIndex index : selectedItems.indexes())
			{
				if (index.row() == i)
				{
					// Obtain current selection
					QItemSelection selectedItems = selectionModel->selection();

					// Select new row
					this->actionTable->selectRow(i);

					// Merge selections
					selectedItems.merge(selectionModel->selection(), QItemSelectionModel::Select);
					selectionModel->clearSelection();
					selectionModel->select(selectedItems, QItemSelectionModel::Select);

					break;
				}
			}
		}
	}

	connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);
}

void ActionEditor::restoreSelection()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return;


	QItemSelectionModel* selectionModel = this->actionTable->selectionModel();

	disconnect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);

	selectionModel->clearSelection();

	// First get locked references to previously selected actions
	QVector<shared_ptr<ActionOnSignal>> previousSelection;

	for (weak_ptr<ActionOnSignal> weakAction : this->latestSelection)
	{
		shared_ptr<ActionOnSignal> lockedAction = weakAction.lock();

		if (lockedAction != nullptr)
			previousSelection.append(lockedAction);
	}

	// Then select back previously selected actions
	for (int i = 0 ; i < actuator->getActions().count() ; i++)
	{
		shared_ptr<ActionOnSignal> action = actuator->getAction(i);

		if (previousSelection.contains(action))
		{
			// Obtain current selection
			QItemSelection selectedItems = selectionModel->selection();

			// Select new row
			this->actionTable->selectRow(i);

			// Merge selections
			selectedItems.merge(selectionModel->selection(), QItemSelectionModel::Select);
			this->actionTable->clearSelection();
			selectionModel->select(selectedItems, QItemSelectionModel::Select);
		}
	}

	connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);
}
