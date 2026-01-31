/*
 * Copyright © 2025-2026 Clément Foucher
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
#include "actiontableview.h"

// Qt classes
#include <QInputEvent>
#include <QHeaderView>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "actiononvariable.h"
#include "variable.h"
#include "actiontablemodel.h"
#include "actiontabletypedelegate.h"
#include "actiontablevaluedelegate.h"
#include "contextmenu.h"
#include "rangeeditordialog.h"


ActionTableView::ActionTableView(componentId_t actuatorId, QWidget* parent) :
	ReorderableTableView(parent)
{
	this->actuatorId = actuatorId;

	// Build table model
	this->tableModel = new ActionTableModel(actuatorId, this);
	this->setModel(this->tableModel);

	// Build delegates
	auto typeColDelegate = new ActionTableTypeDelegate(this);
	this->setItemDelegateForColumn(0, typeColDelegate);
	auto valueColDelegate = new ActionTableValueDelegate(this);
	this->setItemDelegateForColumn(2, valueColDelegate);
}

void ActionTableView::initialize()
{
	this->openPersistentEditors();
}

void ActionTableView::addAction(const QString& variableName)
{
	this->tableModel->addAction(variableName);
}

void ActionTableView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	ReorderableTableView::rowsInserted(parent, start, end);

	uint newRowCount = this->model()->rowCount();
	emit this->rowCountChanged(newRowCount);
}

void ActionTableView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	ReorderableTableView::rowsAboutToBeRemoved(parent, start, end);

	uint currentRowCount = this->model()->rowCount();
	uint newRowCount = currentRowCount - (end-start+1);
	emit this->rowCountChanged(newRowCount);
}

void ActionTableView::keyPressEvent(QKeyEvent* ev)
{
	if (ev->key() == Qt::Key::Key_Delete)
	{
		this->deleteSelectedRows();
	}
}

void ActionTableView::contextMenuEvent(QContextMenuEvent* ev)
{
	uint selectedRowsCount = this->selectionModel()->selectedRows().count();
	if (selectedRowsCount == 0) return;

	shared_ptr<ActionOnVariable> actionActedOn;
	shared_ptr<Variable> variableActedOn;
	if (selectedRowsCount == 1)
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return;

		int actionRank = this->rowAt(ev->pos().y());
		if (actionRank < 0) return;

		auto actuator = machine->getActuatorComponent(this->actuatorId);
		if (actuator == nullptr) return;

		actionActedOn = actuator->getAction(actionRank);
		if (actionActedOn == nullptr) return;

		auto variableActedOnId = actionActedOn->getVariableActedOnId();
		variableActedOn = machine->getVariable(variableActedOnId);
		if (variableActedOn == nullptr) return;


		this->currentMenuRow = actionRank;
	}


	ContextMenu* menu = new ContextMenu();

	// Title
	if (selectedRowsCount == 1)
	{
		menu->addTitle(tr("Edit action on variable") + " <i>" + variableActedOn->getName() + "</i>");
	}
	else
	{
		menu->addTitle(tr("Edit all selected actions"));
	}

	QAction* actionBeingAdded = nullptr;
	QVariant data;

	if (selectedRowsCount == 1)
	{
		if (actionActedOn->isActionValueEditable())
		{
			actionBeingAdded = menu->addAction(tr("Edit value"));
			data.setValue((int)ContextAction::EditValue);
			actionBeingAdded->setData(QVariant());
		}
	}

	if (this->getSelectionCanBeRaised() == true)
	{
		actionBeingAdded = menu->addAction(tr("Move up"));
		data.setValue((int)ContextAction::MoveUp);
		actionBeingAdded->setData(data);
	}

	if (this->getSelectionCanBeLowered() == true)
	{
		actionBeingAdded = menu->addAction(tr("Move down"));
		data.setValue((int)ContextAction::MoveDown);
		actionBeingAdded->setData(data);
	}

	menu->addSeparator();

	if (selectedRowsCount == 1)
	{
		if (variableActedOn->getSize() > 1)
		{
			actionBeingAdded = menu->addAction(tr("Affect whole variable"));
			actionBeingAdded->setCheckable(true);
			if (actionActedOn->getActionRangeL() == -1)
			{
				actionBeingAdded->setChecked(true);
			}
			data.setValue((int)ContextAction::AffectSwitchWhole);
			actionBeingAdded->setData(data);

			actionBeingAdded = menu->addAction(tr("Affect variable single bit"));
			actionBeingAdded->setCheckable(true);
			if ( (actionActedOn->getActionRangeL() != -1) && (actionActedOn->getActionRangeR() == -1) )
			{
				actionBeingAdded->setChecked(true);
			}
			data.setValue((int)ContextAction::AffectSwitchSingle);
			actionBeingAdded->setData(data);

			actionBeingAdded = menu->addAction(tr("Affect variable range"));
			actionBeingAdded->setCheckable(true);
			if ( (actionActedOn->getActionRangeL() != -1) && (actionActedOn->getActionRangeR() != -1) )
			{
				actionBeingAdded->setChecked(true);
			}
			data.setValue((int)ContextAction::AffectSwitchRange);
			actionBeingAdded->setData(data);

			if ( (actionActedOn->getActionRangeL() != -1) || (actionActedOn->getActionRangeR() != -1) )
			{
				if (actionActedOn->getActionRangeR() == -1)
				{
					actionBeingAdded = menu->addAction(tr("Edit affected bit"));
				}
				else
				{
					actionBeingAdded = menu->addAction(tr("Edit range"));
				}

				data.setValue((int)ContextAction::AffectEditRange);
				actionBeingAdded->setData(data);
			}

			menu->addSeparator();
		}

		actionBeingAdded = menu->addAction(tr("Delete action"));
		data.setValue((int)ContextAction::DeleteAction);
		actionBeingAdded->setData(data);
	}
	else // (selectedRowsCount > 1)
	{
		actionBeingAdded = menu->addAction(tr("Delete actions"));
		data.setValue((int)ContextAction::DeleteAction);
		actionBeingAdded->setData(data);
	}

	actionBeingAdded = menu->addAction(tr("Cancel"));
	data.setValue((int)ContextAction::Cancel);
	actionBeingAdded->setData(data);

	// Adjust event position wrt. headers
	QPoint correctedPos;
	correctedPos.setX(ev->pos().x() + this->verticalHeader()->width());
	correctedPos.setY(ev->pos().y() + this->horizontalHeader()->height());

	menu->popup(this->mapToGlobal(correctedPos));


	connect(menu, &QMenu::triggered, this, &ActionTableView::processContextMenuEventHandler);
}

void ActionTableView::processContextMenuEventHandler(QAction* action)
{
	int dataValue = action->data().toInt();

	shared_ptr<ActionOnVariable> actionOnVariable;
	if (this->currentMenuRow >= 0)
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return;

		auto actuator = machine->getActuatorComponent(this->actuatorId);
		if (actuator == nullptr) return;

		actionOnVariable = actuator->getAction(this->currentMenuRow);
		if (actionOnVariable == nullptr) return;
	}

	int oldRangeL;
	int oldRangeR;

	int newRangeL = -1;
	int newRangeR = -1;
	bool setRange = false;

	if ( (dataValue == ContextAction::AffectSwitchWhole)  ||
	     (dataValue == ContextAction::AffectSwitchSingle) ||
	     (dataValue == ContextAction::AffectSwitchRange)  ||
	     (dataValue == ContextAction::AffectEditRange)
	   )
	{
		if (actionOnVariable == nullptr) return;


		oldRangeL = actionOnVariable->getActionRangeL();
		oldRangeR = actionOnVariable->getActionRangeR();
	}

	switch (dataValue)
	{
	case ContextAction::Cancel:
		break;
	case ContextAction::EditValue:
	{
		if (this->currentMenuRow < 0) return;


		auto index = this->tableModel->index(this->currentMenuRow, 2);

		this->setCurrentIndex(index);
		this->edit(index);

		break;
	}
	case ContextAction::DeleteAction:
		// Machine is about to be edited
		machineManager->notifyMachineAboutToBeDiffEdited();

		// Delete actions
		this->deleteSelectedRows();

		// Machine has been edited
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
		// Machine is about to be edited
		machineManager->notifyMachineAboutToBeDiffEdited();

		// Move actions
		this->lowerSelectedRows();

		// Machine has been edited
		machineManager->notifyMachineEdited();
		break;
	case ContextAction::MoveUp:
		// Machine is about to be edited
		machineManager->notifyMachineAboutToBeDiffEdited();

		// Move actions
		this->raiseSelectedRows();

		// Machine has been edited
		machineManager->notifyMachineEdited();
		break;
	case ContextAction::AffectEditRange:
		if (actionOnVariable == nullptr) return;


		this->actionBeingEdited = actionOnVariable;
		this->rangeEditorDialog = new RangeEditorDialog(actionOnVariable->getVariableActedOnId(), oldRangeL, oldRangeR, this);
		connect(this->rangeEditorDialog, &RangeEditorDialog::finished, this, &ActionTableView::rangeEditorClosedEventHandler);

		this->rangeEditorDialog->open();
		break;
	}

	if (setRange == true)
	{
		// Machine is about to be edited
		machineManager->notifyMachineAboutToBeDiffEdited();

		// Change action range
		actionOnVariable->setActionRange(newRangeL, newRangeR);

		// Machine has been edited
		machineManager->notifyMachineEdited();
	}

	this->currentMenuRow = -1;
}

void ActionTableView::rangeEditorClosedEventHandler(int result)
{
	if (this->rangeEditorDialog == nullptr) return;

	if (this->actionBeingEdited == nullptr) return;


	if (result == QDialog::Accepted)
	{
		int newRangeL = this->rangeEditorDialog->getRangeL();
		int newRangeR = this->rangeEditorDialog->getRangeR();

		// Machine is about to be edited
		machineManager->notifyMachineAboutToBeDiffEdited();

		// Change action range
		this->actionBeingEdited->setActionRange(newRangeL, newRangeR);

		// Machine has been edited
		machineManager->notifyMachineEdited();
	}

	this->actionBeingEdited = nullptr;

	delete this->rangeEditorDialog;
	this->rangeEditorDialog = nullptr;
}

void ActionTableView::openPersistentEditors(int firstRow, int lastRow)
{
	if (firstRow == -1) firstRow = 0;
	if (lastRow  == -1) lastRow  = this->tableModel->rowCount()-1;

	for (int row = firstRow ; row <= lastRow ; row++)
	{
		auto typeIndex = this->tableModel->index(row, 0);
		uint32_t actions = typeIndex.data(Qt::EditRole).toUInt();

		// Open a persistent editor for actions that have
		// more than one allowed action type.
		if (std::popcount(actions & 0xFFFF0000) > 1)
		{
			this->openPersistentEditor(typeIndex);
		}
	}
}

void ActionTableView::closePersistentEditors(int firstRow, int lastRow)
{
	if (firstRow == -1) firstRow = 0;
	if (lastRow  == -1) lastRow  = this->tableModel->rowCount()-1;

	for (int row = firstRow ; row <= lastRow ; row++)
	{
		auto typeIndex = this->tableModel->index(row, 0);
		if (this->isPersistentEditorOpen(typeIndex))
		{
			this->closePersistentEditor(typeIndex);
		}
	}
}
