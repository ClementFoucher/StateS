/*
 * Copyright © 2014-2020 Clément Foucher
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
#include "machineactuatorcomponent.h"
#include "actiontypecombobox.h"
#include "contextmenu.h"
#include "machine.h"
#include "StateS_signal.h"
#include "rangeeditordialog.h"
#include "collapsiblewidgetwithtitle.h"
#include "statesexception.h"
#include "actiontablemodel.h"
#include "actiontabledelegate.h"
#include "actiononsignal.h"


ActionEditor::ActionEditor(shared_ptr<MachineActuatorComponent> actuator, QString title, QWidget* parent) :
    QWidget(parent)
{
	this->actuator = actuator;

	QGridLayout* layout = new QGridLayout(this);

	if (title.size() != 0)
	{
		QLabel* actionListTitle = new QLabel(title);
		actionListTitle->setAlignment(Qt::AlignCenter);
		actionListTitle->setWordWrap(true);
		layout->addWidget(actionListTitle, 0, 0, 1, 2);
	}

	this->actionTable = new QTableView();
	ActionTableModel* tableModel = new ActionTableModel(actuator, this->actionTable);
	this->actionTable->setModel(tableModel);
	this->actionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->actionTable->setItemDelegate(new ActionTableDelegate(actuator, this->actionTable));
	this->actionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	connect(this->actionTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);
	connect(tableModel, &QAbstractItemModel::layoutChanged, this, &ActionEditor::tableChangedEventHandler);
	layout->addWidget(this->actionTable, 1, 0, 1, 42);

	this->buttonMoveUp       = new QPushButton("↥");
	this->buttonMoveDown     = new QPushButton("↧");
	this->buttonAddAction    = new QPushButton(tr("Add action"));
	this->buttonRemoveAction = new QPushButton(tr("Remove action"));

	connect(this->buttonMoveUp,       &QPushButton::clicked, this, &ActionEditor::moveSelectedActionsUp);
	connect(this->buttonMoveDown,     &QPushButton::clicked, this, &ActionEditor::moveSelectedActionsDown);
	connect(this->buttonAddAction,    &QPushButton::clicked, this, &ActionEditor::displayAddActionMenu);
	connect(this->buttonRemoveAction, &QPushButton::clicked, this, &ActionEditor::removeSelectedActions);

	layout->addWidget(this->buttonMoveUp,       3, 0,  1, 1);
	layout->addWidget(this->buttonMoveDown,     3, 1,  1, 1);
	layout->addWidget(this->buttonAddAction,    3, 2,  1, 20);
	layout->addWidget(this->buttonRemoveAction, 3, 22, 1, 20);


	this->hintDisplay = new CollapsibleWidgetWithTitle();
	QString hintTitle = tr("Hint:") + " " + tr("Editing actions");

	QString hint;
	hint += "<br />";
	hint += tr("Double-click") + " " + tr("on an affected value") + " " + tr("to edit it.");
	hint += "<br />";
	hint += tr("Right-click") + " " + tr("on a vector signal") + " " + tr("to display range options.");
	hint += "<br />";

	this->hintDisplay->setContent(hintTitle, hint, true);

	layout->addWidget(this->hintDisplay, 4, 0, 1, 42);

	connect(actuator.get(), &MachineActuatorComponent::actionListChangedEvent, this->actionTable, &QTableView::resizeColumnsToContents);

	this->fillFirstColumn();
	this->updateButtonsEnableState();
}

void ActionEditor::keyPressEvent(QKeyEvent* e)
{
	if (! this->actuator.expired())
	{
		bool transmitEvent = true;

		if (e->key() == Qt::Key::Key_Delete)
		{
			this->removeSelectedActions();
			transmitEvent = false;
		}

		if (transmitEvent == true)
			QWidget::keyPressEvent(e);
	}
}

void ActionEditor::keyReleaseEvent(QKeyEvent* e)
{
	if (! this->actuator.expired())
	{
		bool transmitEvent = true;

		if (e->key() == Qt::Key::Key_Delete)
		{
			transmitEvent = false;
		}

		if (transmitEvent == true)
			QWidget::keyReleaseEvent(e);
	}
}

void ActionEditor::contextMenuEvent(QContextMenuEvent* event)
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		QPoint correctedPos = this->actionTable->mapFromParent(event->pos());
		correctedPos.setX(correctedPos.x() - this->actionTable->verticalHeader()->width());
		correctedPos.setY(correctedPos.y() - this->actionTable->horizontalHeader()->height());
		int actionRank = this->actionTable->rowAt(correctedPos.y());

		if (actionRank >= 0)
		{
			try
			{
				shared_ptr<ActionOnSignal> actionActedOn = l_actuator->getAction(actionRank); // Throws StatesException

				ContextMenu* menu = new ContextMenu();
				menu->addTitle(tr("Action on signal") + " <i>" + actionActedOn->getSignalActedOn()->getName() + "</i>");

				QVariant data;
				data.convert(QVariant::Int);
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

				connect(menu, &QMenu::triggered, this, &ActionEditor::treatContextMenuEventHandler);
			}
			catch (StatesException e)
			{
				if ( (e.getSourceClass() != "MachineActuatorComponent") || (e.getEnumValue() != MachineActuatorComponent::out_of_range) )
					throw;
			}
		}
	}
}

void ActionEditor::selectionChangedEventHandler(const QItemSelection&, const QItemSelection&)
{
	if (! this->actuator.expired())
	{
		this->updateButtonsEnableState();

		// Store selection to restore it on model update
		this->latestSelection.clear();

		shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
		if (l_actuator != nullptr)
		{
			QItemSelectionModel* selectionModel = this->actionTable->selectionModel();

			QModelIndexList selectedRows = selectionModel->selectedRows();

			foreach (QModelIndex index, selectedRows)
			{
				this->latestSelection.append(l_actuator->getAction(index.row()));
			}
		}
	}
}

void ActionEditor::fillFirstColumn()
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		if (l_actuator->getActions().count() != 0)
		{
			// Fill column with drop-down list
			for(int i = 0 ; i < this->actionTable->model()->rowCount() ; i++)
			{
				this->actionTable->setIndexWidget(this->actionTable->model()->index(i, 0), new ActionTypeComboBox(l_actuator->getAllowedActionTypes(), l_actuator->getAction(i))); // Throws StatesException - Ignored: list generated from action list
			}
		}
		else
		{
			// Clear potential previous widgets if action count drops to 0
			// (special message displayed)
			this->actionTable->setIndexWidget(this->actionTable->model()->index(0, 0), nullptr);
		}
	}
}

void ActionEditor::displayAddActionMenu() const
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		shared_ptr<Machine> owningMachine = l_actuator->getOwningMachine();

		if (owningMachine != nullptr)
		{
			ContextMenu* menu;

			QList<shared_ptr<Signal>> availableActions = owningMachine->getWrittableSignals();
			if (availableActions.count() != 0)
			{
				menu = new ContextMenu();
				menu->setListStyle();

				foreach(shared_ptr<Signal> var, availableActions)
				{
					menu->addAction(var->getName());
				}

				connect(menu, &QMenu::triggered, this, &ActionEditor::treatAddActionMenuEventHandler);
			}
			else
			{
				menu = ContextMenu::createErrorMenu(tr("No compatible signal!"));
			}

			menu->popup(this->buttonAddAction->mapToGlobal(QPoint(this->buttonAddAction->width(), -menu->sizeHint().height())));
		}
	}
}

void ActionEditor::removeSelectedActions()
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		shared_ptr<Machine> machine = l_actuator->getOwningMachine();
		if (machine != nullptr)
		{
			QModelIndexList indexList = this->actionTable->selectionModel()->selectedRows();
			if (indexList.isEmpty() == false)
			{
				this->latestSelection.clear();
				// Sort the list backwards to avoid index shifting while removing actions
				sort(indexList.rbegin(), indexList.rend());

				machine->beginAtomicEdit();
				for (QModelIndex index : indexList)
				{
					l_actuator->removeAction(index.row()); // Throws StatesException - Ignored: list generated from action list
				}
				machine->endAtomicEdit();
			}
		}
	}
}

void ActionEditor::treatAddActionMenuEventHandler(QAction* action)
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		shared_ptr<Machine> owningMachine = l_actuator->getOwningMachine();
		QString signalName = action->text();

		if (owningMachine != nullptr)
		{
			// Find signal from name
			foreach (shared_ptr<Signal> var, owningMachine->getWrittableSignals())
			{
				if (var->getName() == signalName)
				{
					l_actuator->addAction(var);
					break;
				}
			}
		}
	}
}

void ActionEditor::treatContextMenuEventHandler(QAction* action)
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		int dataValue   = action->data().toInt();
		uint actionRank = (dataValue&0xFFF0)>>8;

		shared_ptr<ActionOnSignal> action = l_actuator->getAction(actionRank); // Throws StatesException - Ignored: list generated from action list

		int oldRangeL = action->getActionRangeL();
		int oldRangeR = action->getActionRangeR();
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
			l_actuator->removeAction(actionRank); // Throws StatesException - Ignored: list generated from action list
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
			l_actuator->changeActionRank(actionRank, actionRank+1);
			break;
		case ContextAction::MoveUp:
			l_actuator->changeActionRank(actionRank, actionRank-1);
			break;
		case ContextAction::AffectEditRange:
			unique_ptr<RangeEditorDialog>rangeEditor = unique_ptr<RangeEditorDialog>(new RangeEditorDialog(action));
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
				action->setActionRange(newRangeL, newRangeR);
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_range) )
			{
				qDebug() << "(ActionEditor:) Warning: Incorrect range was set. Range change ignored.";
			}
			else
				throw;
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
	if (! this->actuator.expired())
	{
		QItemSelectionModel* selectionModel = this->actionTable->selectionModel();

		disconnect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);

		QItemSelection selectedItems = selectionModel->selection();

		selectionModel->clearSelection();

		if ( ! selectedItems.isEmpty() )
		{
			for (int i = 0 ; i < this->actionTable->model()->rowCount() ; i++)
			{
				foreach (QModelIndex index, selectedItems.indexes())
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
}

void ActionEditor::tableChangedEventHandler()
{
	if (! this->actuator.expired())
	{
		this->fillFirstColumn();
		this->restoreSelection();
		this->updateButtonsEnableState();
	}
}

void ActionEditor::restoreSelection()
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		QItemSelectionModel* selectionModel = this->actionTable->selectionModel();

		disconnect(selectionModel, &QItemSelectionModel::selectionChanged, this, &ActionEditor::selectionChangedEventHandler);

		selectionModel->clearSelection();

		// First get locked references to previously selected actions
		QList<shared_ptr<ActionOnSignal>> previousSelection;

		foreach (weak_ptr<ActionOnSignal> weakAction, this->latestSelection)
		{
			shared_ptr<ActionOnSignal> lockedAction = weakAction.lock();

			if (lockedAction != nullptr)
				previousSelection.append(lockedAction);
		}

		// Then select back previously selected actions
		for (int i = 0 ; i < l_actuator->getActions().count() ; i++)
		{
			shared_ptr<ActionOnSignal> action = l_actuator->getAction(i);

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
}

void ActionEditor::updateButtonsEnableState()
{
	this->buttonMoveUp      ->setEnabled(false);
	this->buttonMoveDown    ->setEnabled(false);
	this->buttonRemoveAction->setEnabled(false);

	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		if (l_actuator->getActions().count() != 0)
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
}


void ActionEditor::moveSelectedActionsUp()
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		shared_ptr<Machine> machine = l_actuator->getOwningMachine();
		if (machine != nullptr)
		{
			this->sortSelectionList();

			QList<int> selectedActionsRanks;

			QModelIndexList rows = this->actionTable->selectionModel()->selectedRows();
			foreach (QModelIndex index, rows)
			{
				selectedActionsRanks.append(index.row());
			}

			machine->beginAtomicEdit();
			for (int i = 0 ; i < rows.count() ; i++)
			{
				// Actually lower upper signals rather than raise signal itself

				// Check if previous row is selected
				int currentActionRank = selectedActionsRanks.at(i) ;
				if (currentActionRank != 0)
				{
					bool previousSelected = false;
					foreach(QModelIndex index, this->actionTable->selectionModel()->selectedRows())
					{
						if (index.row() == currentActionRank-1)
						{
							previousSelected = true;
							break;
						}
					}

					if ( previousSelected == false )
					{
						l_actuator->changeActionRank(currentActionRank-1, currentActionRank);
					}
				}
			}
			machine->endAtomicEdit();
		}
	}
}

void ActionEditor::moveSelectedActionsDown()
{
	shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
	if (l_actuator != nullptr)
	{
		shared_ptr<Machine> machine = l_actuator->getOwningMachine();
		if (machine != nullptr)
		{
			this->sortSelectionList();

			QList<int> selectedActionsRanks;

			QModelIndexList rows = this->actionTable->selectionModel()->selectedRows();
			foreach (QModelIndex index, rows)
			{
				selectedActionsRanks.push_front(index.row());
			}

			machine->beginAtomicEdit();
			for (int i = 0 ; i < rows.count() ; i++)
			{
				// Actually raise lower signals rater than lowering signal itself

				// Check if next row is selected
				int currentActionRank = selectedActionsRanks.at(i) ;
				if (currentActionRank != this->actionTable->model()->rowCount()-1)
				{
					bool nextSelected = false;
					foreach(QModelIndex index, this->actionTable->selectionModel()->selectedRows())
					{
						if (index.row() == currentActionRank+1)
						{
							nextSelected = true;
							break;
						}
					}

					if ( nextSelected == false )
					{
						l_actuator->changeActionRank(currentActionRank+1, currentActionRank);
					}
				}
			}
			machine->endAtomicEdit();
		}
	}
}
