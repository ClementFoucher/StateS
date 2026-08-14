/*
 * Copyright © 2025-2026 Clément Foucher
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

// Qt
#include <QHeaderView>
#include <QContextMenuEvent>

// StateS
#include "variabletablemodel.h"
#include "variabletablememorizeddelegate.h"
#include "variabletablenamedelegate.h"
#include "variabletabletypedelegate.h"
#include "variabletablevaluedelegate.h"
#include "contextmenu.h"
#include "typeeditor.h"


VariableTableView::VariableTableView(Machine::VariableNature_t tableNature, QWidget* parent) :
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
			this->columnsRoles[ColumnRole_t::name] = column;
			this->setItemDelegateForColumn(column, new VariableTableNameDelegate(this));
		}
		else if (role == "TYPE")
		{
			this->columnsRoles[ColumnRole_t::type] = column;
			this->setItemDelegateForColumn(column, new VariableTableTypeDelegate(this));
		}
		else if (role == "MEMORIZED")
		{
			this->columnsRoles[ColumnRole_t::memorized] = column;
			this->setItemDelegateForColumn(column, new VariableTableMemorizedDelegate(this));
		}
		else if (role == "VALUE")
		{
			this->columnsRoles[ColumnRole_t::value] = column;
			this->setItemDelegateForColumn(column, new VariableTableValueDelegate(this));
		}
	}

	connect(this->tableModel, &VariableTableModel::refreshPersistentEditorsEvent, this, &VariableTableView::refreshPersistentEditorsEventHandler);
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
		auto index = this->tableModel->index(this->currentMenuRow, this->columnsRoles.value(ColumnRole_t::name));
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
		data.setValue(static_cast<int>(ContextAction_t::raise));
		actionBeingAdded->setData(data);
	}

	if (this->getSelectionCanBeLowered() == true)
	{
		actionBeingAdded = menu->addAction(tr("Move down"));
		data.setValue(static_cast<int>(ContextAction_t::lower));
		actionBeingAdded->setData(data);
	}

	if ( (this->getSelectionCanBeRaised() == true) || (this->getSelectionCanBeLowered() == true) )
	{
		menu->addSeparator();
	}

	if (selectedRowsCount == 1)
	{
		actionBeingAdded = menu->addAction(tr("Rename variable"));
		data.setValue(static_cast<int>(ContextAction_t::rename));
		actionBeingAdded->setData(data);

		auto index = this->tableModel->index(currentMenuRow, this->columnsRoles.value(ColumnRole_t::type));

		if (this->tableModel->data(index, Qt::EditRole).toString().startsWith("BITVECTOR"))
		{
			actionBeingAdded = menu->addAction(tr("Change bit vector size"));
			data.setValue(static_cast<int>(ContextAction_t::resizeBitVector));
			actionBeingAdded->setData(data);
		}

		if (this->tableModel->data(index, Qt::EditRole) != "BOOLEAN")
		{
			actionBeingAdded = menu->addAction(tr("Change variable value"));
			data.setValue(static_cast<int>(ContextAction_t::changeValue));
			actionBeingAdded->setData(data);
		}

		menu->addSeparator();

		actionBeingAdded = menu->addAction(tr("Delete variable"));
		data.setValue(static_cast<int>(ContextAction_t::deleteVar));
		actionBeingAdded->setData(data);
	}
	else // (selectedRowsCount > 1)
	{
		actionBeingAdded = menu->addAction(tr("Delete variables"));
		data.setValue(static_cast<int>(ContextAction_t::deleteVar));
		actionBeingAdded->setData(data);
	}

	menu->addSeparator();

	actionBeingAdded = menu->addAction(tr("Cancel"));
	data.setValue(static_cast<int>(ContextAction_t::cancel));
	actionBeingAdded->setData(data);

	// Adjust event position wrt. headers
	QPoint correctedPos;
	correctedPos.setX(event->pos().x() + this->verticalHeader()->width());
	correctedPos.setY(event->pos().y() + this->horizontalHeader()->height());

	menu->popup(this->mapToGlobal(correctedPos));

	connect(menu, &QMenu::triggered, this, &VariableTableView::processMenuEventHandler);
}

void VariableTableView::openPersistentEditors(int firstRow, int firstColumn, int lastRow, int lastColumn)
{
	if (firstRow == -1) firstRow = 0;
	if (lastRow  == -1) lastRow  = this->tableModel->rowCount()-1;
	if (firstColumn == -1) firstColumn = 0;
	if (lastColumn  == -1) lastColumn  = this->tableModel->columnCount()-1;

	for (int row = firstRow ; row <= lastRow ; row++)
	{
		for (int col = firstColumn ; col <= lastColumn ; col++)
		{
			if (col == this->columnsRoles[ColumnRole_t::type])
			{
				auto typeIndex = this->tableModel->index(row, col);
				this->openPersistentEditor(typeIndex);
			}
			else if ( (this->columnsRoles.contains(ColumnRole_t::memorized) == true) && (col == this->columnsRoles[ColumnRole_t::memorized]) )
			{
				auto memIndex = this->tableModel->index(row, col);
				this->openPersistentEditor(memIndex);
			}
			else if (col == this->columnsRoles[ColumnRole_t::value])
			{
				auto valIndex = this->tableModel->index(row, col);
				if (this->tableModel->data(valIndex, Qt::EditRole).toString().startsWith("BOOLEAN") == true)
				{
					this->openPersistentEditor(valIndex);
				}
			}
		}
	}
}

void VariableTableView::processMenuEventHandler(QAction* action)
{
	ContextAction_t dataValue = static_cast<ContextAction_t>(action->data().toInt());

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
		auto col = this->columnsRoles.value(ColumnRole_t::name);
		auto index = this->tableModel->index(this->currentMenuRow, col);
		this->setCurrentIndex(index);
		this->edit(index);
	}
	break;
	case ContextAction_t::changeValue:
	{
		auto col = this->columnsRoles.value(ColumnRole_t::value);
		auto index = this->tableModel->index(this->currentMenuRow, col);
		this->setCurrentIndex(index);
		this->edit(index);
	}
	break;
	case ContextAction_t::resizeBitVector:
	{
		auto col = this->columnsRoles.value(ColumnRole_t::type);
		auto index = this->tableModel->index(currentMenuRow, col);
		auto editor = dynamic_cast<TypeEditor*>(this->indexWidget(index));
		if (editor != nullptr)
		{
			this->setCurrentIndex(index);
			editor->triggerEditBitVectorSize();
		}
	}
	break;
	}
}

void VariableTableView::refreshPersistentEditorsEventHandler()
{
	this->closePersistentEditors();
	this->openPersistentEditors();
}
