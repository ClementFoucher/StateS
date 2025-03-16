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
#include "variabletablenamedelegate.h"
#include "variabletablesizedelegate.h"
#include "variabletablevaluedelegate.h"
#include "contextmenu.h"


VariableTableView::VariableTableView(VariableNature_t tableNature, QWidget* parent) :
	QTableView(parent)
{
	// Configure table
	this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Build table model
	this->tableModel = new VariableTableModel(tableNature, this);
	this->setModel(this->tableModel);

	// Strangely enough, unlike rows insertion/removal and other changes,
	// rows move doesn't seem to have its own handler (outside QAbstractItemModelPrivate)
	connect(this->tableModel, &VariableTableModel::rowsMovedEvent, this, &VariableTableView::rowsMovedEventHandler);

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
		else if (role == "VALUE")
		{
			this->columnsRoles[ColumnRole::value] = column;
			this->setItemDelegateForColumn(column, new VariableTableValueDelegate(this));
		}
	}
}

void VariableTableView::addNewVariable()
{
	this->tableModel->insertRow(this->tableModel->rowCount());
}

void VariableTableView::deleteSelectedVariables()
{
	auto selectedRanksBlocks = this->getSelectedRanksBlocks();

	// Deleting a block will mess with ranks of lower blocks:
	// delete blocks from bottom to top
	std::reverse(selectedRanksBlocks.begin(), selectedRanksBlocks.end());

	for (auto block : as_const(selectedRanksBlocks))
	{
		this->tableModel->removeRows(block.first, block.second);
	}
}

void VariableTableView::raiseSelectedVariables()
{
	const auto selectedRanksBlocks = this->getSelectedRanksBlocks();
	for (auto rankBlock : selectedRanksBlocks)
	{
		// Do not raise block if it is already at the top of the list
		if (rankBlock.first != 0)
		{
			auto parent = this->tableModel->index(rankBlock.first, 0).parent();
			this->tableModel->moveRows(parent, rankBlock.first, rankBlock.second, parent, rankBlock.first-1);
		}
	}
}

void VariableTableView::lowerSelectedVariables()
{
	const auto selectedRanksBlocks = this->getSelectedRanksBlocks();
	for (auto rankBlock : selectedRanksBlocks)
	{
		// Do not lower block if it is already at the bottom of the list
		if ( (rankBlock.first + rankBlock.second) != this->tableModel->rowCount())
		{
			auto parent = this->tableModel->index(rankBlock.first, 0).parent();
			this->tableModel->moveRows(parent, rankBlock.first, rankBlock.second, parent, rankBlock.first+1);
		}
	}
}

bool VariableTableView::getSelectedVariablesCanBeDeleted()
{
	return this->canSelectedVariablesBeDeleted;
}

bool VariableTableView::getSelectedVariablesCanBeRaised()
{
	return this->canSelectedVariablesBeRaised;
}

bool VariableTableView::getSelectedVariablesCanBeLowered()
{
	return this->canSelectedVariablesBeLowered;
}

void VariableTableView::contextMenuEvent(QContextMenuEvent* event)
{
	auto selectedRanks = this->getSelectedRowsRanks();
	if (selectedRanks.count() == 0) return;


	event->accept();
	auto menu = new ContextMenu();

	QAction* actionBeingAdded = nullptr;
	QVariant data;

	// Title
	if (selectedRanks.count() == 1)
	{
		auto index = this->tableModel->index(selectedRanks.at(0), this->columnsRoles.value(ColumnRole::name));
		auto variableName = this->tableModel->data(index, Qt::DisplayRole).toString();
		menu->addTitle(tr("Action on variable") + " <i>" + variableName + "</i>");
	}
	else
	{
		menu->addTitle(tr("Action on all selected variables"));
	}

	// Items
	if (this->canSelectedVariablesBeRaised == true)
	{
		actionBeingAdded = menu->addAction(tr("Move up"));
		data.setValue((int)ContextAction_t::raise);
		actionBeingAdded->setData(data);
	}

	if (this->canSelectedVariablesBeLowered == true)
	{
		actionBeingAdded = menu->addAction(tr("Move down"));
		data.setValue((int)ContextAction_t::lower);
		actionBeingAdded->setData(data);
	}

	if ( (this->canSelectedVariablesBeRaised == true) || (this->canSelectedVariablesBeLowered == true) )
	{
		menu->addSeparator();
	}

	if (selectedRanks.count() == 1)
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
	else
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

void VariableTableView::resizeEvent(QResizeEvent* event)
{
	QTableView::resizeEvent(event);

	auto selectedRows = this->selectionModel()->selectedRows();
	if (selectedRows.isEmpty() == false)
	{
		this->scrollTo(selectedRows.at(0));
	}
}

QList<int> VariableTableView::getSelectedRowsRanks() const
{
	const auto rows = this->selectionModel()->selectedRows();

	QList<int> variablesRanks;
	for (QModelIndex index : rows)
	{
		variablesRanks.append(index.row());
	}

	return variablesRanks;
}

QList<QPair<int, int>> VariableTableView::getSelectedRanksBlocks() const
{
	QList<QPair<int, int>> list;

	auto selectedRows = this->getSelectedRowsRanks();
	auto rowsCount = this->tableModel->rowCount();

	int currentRow = 0;
	while (currentRow < rowsCount)
	{
		if (selectedRows.contains(currentRow) == true)
		{
			int blockEnd;

			for (blockEnd = currentRow+1 ; blockEnd < rowsCount ; blockEnd++)
			{
				if (selectedRows.contains(blockEnd) == false)
				{
					break;
				}
			}
			list.append(QPair<int, int>(currentRow, blockEnd-currentRow));
			currentRow = blockEnd;
		}
		else
		{
			currentRow++;
		}
	}

	return list;
}

void VariableTableView::updateSelectionFlags()
{
	this->canSelectedVariablesBeRaised  = false;
	this->canSelectedVariablesBeLowered = false;
	this->canSelectedVariablesBeDeleted = false;

	auto selectedRowsBlocks = this->getSelectedRanksBlocks();
	if (selectedRowsBlocks.count() != 0)
	{
		this->canSelectedVariablesBeDeleted = true;

		if (selectedRowsBlocks.count() > 1)
		{
			// Discontinuous selection can always be moved up or down
			this->canSelectedVariablesBeRaised  = true;
			this->canSelectedVariablesBeLowered = true;
		}
		else
		{
			// If single group, check if at top or at bottom
			if (selectedRowsBlocks.at(0).first != 0)
			{
				this->canSelectedVariablesBeRaised  = true;
			}

			if (selectedRowsBlocks.last().first + selectedRowsBlocks.last().second < this->model()->rowCount())
			{
				this->canSelectedVariablesBeLowered = true;
			}
		}
	}

	emit this->variablesSelectionChangedEvent();
}

void VariableTableView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTableView::selectionChanged(selected, deselected);

	this->updateSelectionFlags();
}

void VariableTableView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles)
{
	QTableView::dataChanged(topLeft, bottomRight, roles);

	this->updateSelectionFlags();
}

void VariableTableView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	QTableView::rowsInserted(parent, start, end);

	// We only insert rows one at a time, at the bottom of the list,
	// thus select and edit lowest row.
	this->selectRow(end);
	this->edit(this->tableModel->index(end, 0));
}

void VariableTableView::rowsMovedEventHandler()
{
	this->updateSelectionFlags();
}

void VariableTableView::processMenuEventHandler(QAction* action)
{
	ContextAction_t dataValue = ContextAction_t(action->data().toInt());

	switch (dataValue)
	{
	case ContextAction_t::cancel:
		break;
	case ContextAction_t::deleteVar:
		this->deleteSelectedVariables();
		break;
	case ContextAction_t::raise:
		this->raiseSelectedVariables();
		break;
	case ContextAction_t::lower:
		this->lowerSelectedVariables();
		break;
	case ContextAction_t::rename:
		{
			auto row = this->getSelectedRowsRanks().at(0);
			auto col = this->columnsRoles.value(ColumnRole::name);
			this->edit(this->tableModel->index(row, col));
		}
		break;
	case ContextAction_t::changeValue:
		{
			auto row = this->getSelectedRowsRanks().at(0);
			auto col = this->columnsRoles.value(ColumnRole::value);
			this->edit(this->tableModel->index(row, col));
		}
		break;
	case ContextAction_t::resizeBitVector:
		{
			auto row = this->getSelectedRowsRanks().at(0);
			auto col = this->columnsRoles.value(ColumnRole::size);
			this->edit(this->tableModel->index(row, col));
		}
		break;
	}
}
