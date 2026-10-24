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

// Qt
#include <QInputEvent>
#include <QHeaderView>

// StateS
#include "variable.h"
#include "actiontypedelegate.h"
#include "machinevaluedelegate.h"
#include "contextmenu.h"
#include "rangeeditordialog.h"


ActionTableView::ActionTableView(ComponentId actuatorId, QWidget* parent) :
	ReorderableTableView(parent)
{
	this->actuatorId = actuatorId;

	// Build table model
	this->tableModel = new ActionTableModel(actuatorId, this);
	this->setModel(this->tableModel);

	// Determine columns roles and build delegates accordingly
	for (int column = 0 ; column < this->tableModel->columnCount() ; column++)
	{
		QString role = this->tableModel->headerData(column, Qt::Horizontal, Qt::UserRole).toString();
		if (role == "VARIABLENAME")
		{
			this->columnsRoles[ActionTableModel::ColumnRole_t::variableName] = column;
		}
		else if (role == "ACTIONTYPE")
		{
			this->columnsRoles[ActionTableModel::ColumnRole_t::actionType] = column;
			this->setItemDelegateForColumn(this->columnsRoles[ActionTableModel::ColumnRole_t::actionType], new ActionTypeDelegate(this));
		}
		else if (role == "ACTIONVALUE")
		{
			this->columnsRoles[ActionTableModel::ColumnRole_t::actionValue] = column;
			this->setItemDelegateForColumn(this->columnsRoles[ActionTableModel::ColumnRole_t::actionValue], new MachineValueDelegate(this));
		}
	}

	// Connect signals
	connect(this->tableModel, &ActionTableModel::refreshPersistentEditorsEvent, this, &ActionTableView::refreshPersistentEditorsEventHandler);
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
	emit this->rowCountChangedEvent(newRowCount);
}

void ActionTableView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	ReorderableTableView::rowsAboutToBeRemoved(parent, start, end);

	uint currentRowCount = this->model()->rowCount();
	uint newRowCount = currentRowCount - (end-start+1);
	emit this->rowCountChangedEvent(newRowCount);
}

void ActionTableView::keyPressEvent(QKeyEvent* ev)
{
	if (ev->key() == Qt::Key::Key_Delete)
	{
		this->deleteSelectedRows();
	}
	else
	{
		ReorderableTableView::keyPressEvent(ev);
	}
}

void ActionTableView::contextMenuEvent(QContextMenuEvent* ev)
{
	uint selectedRowsCount = this->selectionModel()->selectedRows().count();
	if (selectedRowsCount == 0) return;

	std::shared_ptr<const Variable> variableActedOn;
	if (selectedRowsCount == 1)
	{
		int actionRank = this->rowAt(ev->pos().y());
		if (actionRank < 0) return;

		variableActedOn = this->tableModel->getVariable(actionRank);
		if (variableActedOn == nullptr) return;


		this->currentMenuRow = actionRank;
	}


	auto menu = new ContextMenu();

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
		auto actionValueIndex = this->tableModel->index(this->currentMenuRow, this->columnsRoles[ActionTableModel::ColumnRole_t::actionValue]);
		auto flags = this->tableModel->flags(actionValueIndex);
		if ( (flags & Qt::ItemIsEditable) != 0)
		{
			actionBeingAdded = menu->addAction(tr("Edit value"));
			data.setValue(static_cast<int>(ContextAction_t::EditValue));
			actionBeingAdded->setData(data);
		}
	}

	if (this->getSelectionCanBeRaised() == true)
	{
		actionBeingAdded = menu->addAction(tr("Move up"));
		data.setValue(static_cast<int>(ContextAction_t::MoveUp));
		actionBeingAdded->setData(data);
	}

	if (this->getSelectionCanBeLowered() == true)
	{
		actionBeingAdded = menu->addAction(tr("Move down"));
		data.setValue(static_cast<int>(ContextAction_t::MoveDown));
		actionBeingAdded->setData(data);
	}

	menu->addSeparator();

	if (selectedRowsCount == 1)
	{
		if ( (variableActedOn->getType() == MachineValue::Type_t::bitVector) && (variableActedOn->getInitialValue().getBitVectorValue().getSize() > 1) )
		{
			actionBeingAdded = menu->addAction(tr("Assign whole variable"));
			actionBeingAdded->setCheckable(true);
			if (this->tableModel->getActionRangeL(this->currentMenuRow) == -1)
			{
				actionBeingAdded->setChecked(true);
			}
			data.setValue(static_cast<int>(ContextAction_t::BitVectorAssignWhole));
			actionBeingAdded->setData(data);

			actionBeingAdded = menu->addAction(tr("Assign a single bit"));
			actionBeingAdded->setCheckable(true);
			if ( (this->tableModel->getActionRangeL(this->currentMenuRow) != -1) && (this->tableModel->getActionRangeR(this->currentMenuRow) == -1) )
			{
				actionBeingAdded->setChecked(true);
			}
			data.setValue(static_cast<int>(ContextAction_t::BitVectorAssignSingleBit));
			actionBeingAdded->setData(data);

			actionBeingAdded = menu->addAction(tr("Assign a sub-range"));
			actionBeingAdded->setCheckable(true);
			if ( (this->tableModel->getActionRangeL(this->currentMenuRow) != -1) && (this->tableModel->getActionRangeR(this->currentMenuRow) != -1) )
			{
				actionBeingAdded->setChecked(true);
			}
			data.setValue(static_cast<int>(ContextAction_t::BitVectorAssignSubRange));
			actionBeingAdded->setData(data);

			if ( (this->tableModel->getActionRangeL(this->currentMenuRow) != -1) || (this->tableModel->getActionRangeR(this->currentMenuRow) != -1) )
			{
				if (this->tableModel->getActionRangeR(this->currentMenuRow) == -1)
				{
					actionBeingAdded = menu->addAction(tr("Edit assigned bit"));
				}
				else
				{
					actionBeingAdded = menu->addAction(tr("Edit sub-range"));
				}

				data.setValue(static_cast<int>(ContextAction_t::BitVectorEditAssignRange));
				actionBeingAdded->setData(data);
			}

			menu->addSeparator();
		}

		actionBeingAdded = menu->addAction(tr("Delete action"));
		data.setValue(static_cast<int>(ContextAction_t::DeleteAction));
		actionBeingAdded->setData(data);
	}
	else // (selectedRowsCount > 1)
	{
		actionBeingAdded = menu->addAction(tr("Delete actions"));
		data.setValue(static_cast<int>(ContextAction_t::DeleteAction));
		actionBeingAdded->setData(data);
	}

	actionBeingAdded = menu->addAction(tr("Cancel"));
	data.setValue(static_cast<int>(ContextAction_t::Cancel));
	actionBeingAdded->setData(data);

	// Adjust event position wrt. headers
	QPoint correctedPos;
	correctedPos.setX(ev->pos().x() + this->verticalHeader()->width());
	correctedPos.setY(ev->pos().y() + this->horizontalHeader()->height());

	menu->popup(this->mapToGlobal(correctedPos));


	connect(menu, &QMenu::triggered, this, &ActionTableView::processContextMenuEventHandler);
}

void ActionTableView::openPersistentEditors(int firstRow, int firstColumn, int lastRow, int lastColumn)
{
	if (firstRow == -1) firstRow = 0;
	if (lastRow  == -1) lastRow  = this->tableModel->rowCount()-1;
	if (firstColumn == -1) firstColumn = 0;
	if (lastColumn  == -1) lastColumn  = this->tableModel->columnCount()-1;

	for (int row = firstRow ; row <= lastRow ; row++)
	{
		for (int col = firstColumn ; col <= lastColumn ; col++)
		{
			if (col == this->columnsRoles[ActionTableModel::ColumnRole_t::actionType])
			{
				auto typeIndex = this->tableModel->index(row, col);
				uint32_t actions = typeIndex.data(Qt::EditRole).toUInt();

				// Open a persistent editor for actions that have
				// more than one allowed action type.
				if (std::popcount(actions & 0xFFFF0000) > 1)
				{
					this->openPersistentEditor(typeIndex);
				}
			}
			else if (col == this->columnsRoles[ActionTableModel::ColumnRole_t::actionValue])
			{
				auto valueIndex = this->tableModel->index(row, col);
				if (this->tableModel->data(valueIndex, Qt::EditRole).toString().startsWith("BOOLEAN"))
				{
					this->openPersistentEditor(valueIndex);
				}
			}
		}
	}
}

void ActionTableView::processContextMenuEventHandler(QAction* action)
{
	auto dataValue = static_cast<ContextAction_t>(action->data().toInt());
	switch (dataValue)
	{
	case ContextAction_t::Cancel:
		break;
	case ContextAction_t::EditValue:
	{
		if (this->currentMenuRow < 0) return;


		auto index = this->tableModel->index(this->currentMenuRow, this->columnsRoles[ActionTableModel::ColumnRole_t::actionValue]);

		this->setCurrentIndex(index);
		this->edit(index);

		this->currentMenuRow = -1;
		break;
	}
	case ContextAction_t::DeleteAction:
		this->deleteSelectedRows();
		break;
	case ContextAction_t::BitVectorAssignWhole:
		if (this->currentMenuRow < 0) return;


		this->tableModel->setRange(this->currentMenuRow, -1, -1);

		this->currentMenuRow = -1;
		break;
	case ContextAction_t::BitVectorAssignSingleBit:
		if (this->currentMenuRow < 0) return;


		this->setAssignSingleBit(static_cast<uint>(this->currentMenuRow));

		this->currentMenuRow = -1;
		break;
	case ContextAction_t::BitVectorAssignSubRange:
		if (this->currentMenuRow < 0) return;


		this->setAssignRange(static_cast<uint>(this->currentMenuRow));

		this->currentMenuRow = -1;
		break;
	case ContextAction_t::BitVectorEditAssignRange:
		if (this->currentMenuRow < 0) return;


		this->openRangeEditor(static_cast<uint>(this->currentMenuRow));
		break;
	case ContextAction_t::MoveDown:
		this->lowerSelectedRows();
		break;
	case ContextAction_t::MoveUp:
		this->raiseSelectedRows();
		break;
	}
}

void ActionTableView::rangeEditorClosedEventHandler(int result)
{
	if (this->rangeEditorDialog == nullptr) return;

	if (this->currentMenuRow < 0) return;


	if (result == QDialog::Accepted)
	{
		int newRangeL = this->rangeEditorDialog->getRangeL();
		int newRangeR = this->rangeEditorDialog->getRangeR();

		this->tableModel->setRange(this->currentMenuRow, newRangeL, newRangeR);
	}

	this->currentMenuRow = -1;

	delete this->rangeEditorDialog;
	this->rangeEditorDialog = nullptr;
}

void ActionTableView::refreshPersistentEditorsEventHandler()
{
	this->closePersistentEditors();
	this->openPersistentEditors();
}

void ActionTableView::setAssignSingleBit(uint row)
{
	int oldRangeL = this->tableModel->getActionRangeL(row);
	int oldRangeR = this->tableModel->getActionRangeR(row);

	if (oldRangeL == -1)
	{
		this->tableModel->setRange(row, 0, -1);
	}
	else if (oldRangeR != -1)
	{
		this->tableModel->setRange(row, oldRangeL, -1);
	}
}

void ActionTableView::setAssignRange(uint row)
{
	int oldRangeL = this->tableModel->getActionRangeL(row);
	int oldRangeR = this->tableModel->getActionRangeR(row);

	int newRangeL = -1;
	int newRangeR = -1;
	if (oldRangeL == -1)
	{
		newRangeL = 1;
		newRangeR = 0;
	}
	else if  (oldRangeR == -1)
	{
		if (oldRangeL != 0)
		{
			newRangeL = oldRangeL;
			newRangeR = 0;
		}
		else
		{
			newRangeL = 1;
			newRangeR = 0;
		}
	}

	this->tableModel->setRange(row, newRangeL, newRangeR);
}

void ActionTableView::openRangeEditor(uint row)
{
	auto variable = this->tableModel->getVariable(row);
	int oldRangeL = this->tableModel->getActionRangeL(row);
	int oldRangeR = this->tableModel->getActionRangeR(row);

	this->rangeEditorDialog = new RangeEditorDialog(variable->getId(), oldRangeL, oldRangeR, this);
	connect(this->rangeEditorDialog, &RangeEditorDialog::finished, this, &ActionTableView::rangeEditorClosedEventHandler);

	this->rangeEditorDialog->open();
}
