/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "variabletableview.h"

// Qt classes
#include <QHeaderView>
#include <QContextMenuEvent>

// StateS classes
#include "variabletablemodel.h"
#include "variabletablememorizeddelegate.h"
#include "variabletablenamedelegate.h"
#include "variabletablesizedelegate.h"
#include "variabletablevaluedelegate.h"
#include "contextmenu.h"


VariableTableView::VariableTableView(VariableNature_t tableNature, QWidget* parent) :
	ReorderableTableView(parent)
{
	// Build table model
	this->tableModel = new VariableTableModel(tableNature, this);
	this->setModel(this->tableModel);

	// Determine columns roles and build delegates accordingly
	for (int column = 0 ; column < this->tableModel->columnCount() ; column++)
	{
		QString role = this->tableModel->headerData(column, Qt::Horizontal, Qt::UserRole).toString();
		if (role == "NAME")
		{
			this->columnsRoles[ColumnRole::name] = column;
			this->setItemDelegateForColumn(column, new VariableTableNameDelegate(this));
		}
		else if (role == "SIZE")
		{
			this->columnsRoles[ColumnRole::size] = column;
			this->setItemDelegateForColumn(column, new VariableTableSizeDelegate(this));
		}
		else if (role == "MEMORIZED")
		{
			this->columnsRoles[ColumnRole::memorized] = column;
			this->setItemDelegateForColumn(column, new VariableTableMemorizedDelegate(this));
		}
		else if (role == "VALUE")
		{
			this->columnsRoles[ColumnRole::value] = column;
			this->setItemDelegateForColumn(column, new VariableTableValueDelegate(this));
		}
	}
}

void VariableTableView::initialize()
{
	this->openPersistentEditors();
}

void VariableTableView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	ReorderableTableView::rowsInserted(parent, start, end);

	// We only insert rows one at a time, at the bottom of the list,
	// thus select and edit lowest row.
	this->selectRow(end);
	this->edit(this->model()->index(end, 0));
}

void VariableTableView::contextMenuEvent(QContextMenuEvent* event)
{
	uint selectedRowsCount = this->selectionModel()->selectedRows().count();
	if (selectedRowsCount == 0) return;


	event->accept();
	auto menu = new ContextMenu();

	QAction* actionBeingAdded = nullptr;
	QVariant data;

	// Title
	if (selectedRowsCount == 1)
	{
		this->currentMenuRow = this->rowAt(event->pos().y());
		auto index = this->tableModel->index(this->currentMenuRow, this->columnsRoles.value(ColumnRole::name));
		auto variableName = this->tableModel->data(index, Qt::DisplayRole).toString();
		menu->addTitle(tr("Edit variable") + " <i>" + variableName + "</i>");
	}
	else
	{
		menu->addTitle(tr("Edit all selected variables"));
	}

	// Items
	if (this->getSelectionCanBeRaised() == true)
	{
		actionBeingAdded = menu->addAction(tr("Move up"));
		data.setValue((int)ContextAction_t::raise);
		actionBeingAdded->setData(data);
	}

	if (this->getSelectionCanBeLowered() == true)
	{
		actionBeingAdded = menu->addAction(tr("Move down"));
		data.setValue((int)ContextAction_t::lower);
		actionBeingAdded->setData(data);
	}

	if ( (this->getSelectionCanBeRaised() == true) || (this->getSelectionCanBeLowered() == true) )
	{
		menu->addSeparator();
	}

	if (selectedRowsCount == 1)
	{
		actionBeingAdded = menu->addAction(tr("Rename variable"));
		data.setValue((int)ContextAction_t::rename);
		actionBeingAdded->setData(data);

		actionBeingAdded = menu->addAction(tr("Change bit vector size"));
		data.setValue((int)ContextAction_t::resizeBitVector);
		actionBeingAdded->setData(data);

		actionBeingAdded = menu->addAction(tr("Change variable value"));
		data.setValue((int)ContextAction_t::changeValue);
		actionBeingAdded->setData(data);

		menu->addSeparator();

		actionBeingAdded = menu->addAction(tr("Delete variable"));
		data.setValue((int)ContextAction_t::deleteVar);
		actionBeingAdded->setData(data);
	}
	else // (selectedRowsCount > 1)
	{
		actionBeingAdded = menu->addAction(tr("Delete variables"));
		data.setValue((int)ContextAction_t::deleteVar);
		actionBeingAdded->setData(data);
	}

	menu->addSeparator();

	actionBeingAdded = menu->addAction(tr("Cancel"));
	data.setValue((int)ContextAction_t::cancel);
	actionBeingAdded->setData(data);

	// Adjust event position wrt. headers
	QPoint correctedPos;
	correctedPos.setX(event->pos().x() + this->verticalHeader()->width());
	correctedPos.setY(event->pos().y() + this->horizontalHeader()->height());

	menu->popup(this->mapToGlobal(correctedPos));

	connect(menu, &QMenu::triggered, this, &VariableTableView::processMenuEventHandler);
}

void VariableTableView::processMenuEventHandler(QAction* action)
{
	ContextAction_t dataValue = ContextAction_t(action->data().toInt());

	switch (dataValue)
	{
	case ContextAction_t::cancel:
		break;
	case ContextAction_t::deleteVar:
		this->deleteSelectedRows();
		break;
	case ContextAction_t::raise:
		this->raiseSelectedRows();
		break;
	case ContextAction_t::lower:
		this->lowerSelectedRows();
		break;
	case ContextAction_t::rename:
	{
		auto col = this->columnsRoles.value(ColumnRole::name);
		this->edit(this->tableModel->index(this->currentMenuRow, col));
	}
	break;
	case ContextAction_t::changeValue:
	{
		auto col = this->columnsRoles.value(ColumnRole::value);
		this->edit(this->tableModel->index(this->currentMenuRow, col));
	}
	break;
	case ContextAction_t::resizeBitVector:
	{
		auto col = this->columnsRoles.value(ColumnRole::size);
		this->edit(this->tableModel->index(this->currentMenuRow, col));
	}
	break;
	}
}

void VariableTableView::openPersistentEditors(int firstRow, int lastRow)
{
	if (this->columnsRoles.contains(ColumnRole::memorized))
	{
		if (firstRow == -1) firstRow = 0;
		if (lastRow  == -1) lastRow  = this->tableModel->rowCount()-1;

		for (int row = firstRow ; row <= lastRow ; row++)
		{
			auto memIndex = this->tableModel->index(row, this->columnsRoles.value(ColumnRole::memorized));
			this->openPersistentEditor(memIndex);
		}
	}
}

void VariableTableView::closePersistentEditors(int firstRow, int lastRow)
{
	if (this->columnsRoles.contains(ColumnRole::memorized))
	{
		if (firstRow == -1) firstRow = 0;
		if (lastRow  == -1) lastRow  = this->tableModel->rowCount()-1;

		for (int row = firstRow ; row <= lastRow ; row++)
		{
			auto index = this->tableModel->index(row, this->columnsRoles.value(ColumnRole::memorized));
			if (this->isPersistentEditorOpen(index) == true)
			{
				this->closePersistentEditor(index);
			}
		}
	}
}
