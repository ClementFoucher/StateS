/*
 * Copyright © 2025 Clément Foucher
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
#include "reorderabletableview.h"

// Qt classes
#include <QAbstractTableModel>
#include <QHeaderView>


ReorderableTableView::ReorderableTableView(QWidget* parent) :
	QTableView(parent)
{
	this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void ReorderableTableView::addNewRow()
{
	this->model()->insertRow(this->model()->rowCount());
}

void ReorderableTableView::deleteSelectedRows()
{
	auto selectedRanksBlocks = this->getSelectedRanksBlocks();

	// Deleting a block will mess with ranks of lower blocks:
	// delete blocks from bottom to top
	std::reverse(selectedRanksBlocks.begin(), selectedRanksBlocks.end());

	for (auto block : as_const(selectedRanksBlocks))
	{
		this->model()->removeRows(block.first, block.second);
	}
}

void ReorderableTableView::raiseSelectedRows()
{
	const auto selectedRanksBlocks = this->getSelectedRanksBlocks();
	for (auto rankBlock : selectedRanksBlocks)
	{
		// Do not raise block if it is already at the top of the list
		if (rankBlock.first != 0)
		{
			auto parent = this->model()->index(rankBlock.first, 0).parent();
			this->model()->moveRows(parent, rankBlock.first, rankBlock.second, parent, rankBlock.first-1);
		}
	}
}

void ReorderableTableView::lowerSelectedRows()
{
	const auto selectedRanksBlocks = this->getSelectedRanksBlocks();
	for (auto rankBlock : selectedRanksBlocks)
	{
		// Do not lower block if it is already at the bottom of the list
		if ( (rankBlock.first + rankBlock.second) != this->model()->rowCount())
		{
			auto parent = this->model()->index(rankBlock.first, 0).parent();
			this->model()->moveRows(parent, rankBlock.first, rankBlock.second, parent, rankBlock.first+1);
		}
	}
}

bool ReorderableTableView::getSelectionCanBeDeleted()
{
	return this->canSelectionBeDeleted;
}

bool ReorderableTableView::getSelectionCanBeRaised()
{
	return this->canSelectionBeRaised;
}

bool ReorderableTableView::getSelectionCanBeLowered()
{
	return this->canSelectionBeLowered;
}

void ReorderableTableView::setModel(QAbstractItemModel* model)
{
	QTableView::setModel(model);

	// Strangely enough, unlike rows insertion/removal and other changes,
	// rows move doesn't seem to have its own handler:
	// QTableView:::rowMoved exists, but is not virtual so can't be overriden.
	// We thus have to provide our own handlers and connect to adequate signals in model
	connect(model, &QAbstractTableModel::rowsAboutToBeMoved, this, &ReorderableTableView::rowsAboutToMoveEventHandler);
	connect(model, &QAbstractTableModel::rowsMoved,          this, &ReorderableTableView::rowsMovedEventHandler);
}

void ReorderableTableView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTableView::selectionChanged(selected, deselected);

	this->updateSelectionFlags();
}

void ReorderableTableView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles)
{
	this->closePersistentEditors(topLeft.row(), bottomRight.row());

	QTableView::dataChanged(topLeft, bottomRight, roles);

	this->openPersistentEditors(topLeft.row(), bottomRight.row());
}

void ReorderableTableView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	QTableView::rowsInserted(parent, start, end);

	this->openPersistentEditors(start, end);
}

void ReorderableTableView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	this->closePersistentEditors(start, end);

	QTableView::rowsAboutToBeRemoved(parent, start, end);
}

void ReorderableTableView::resizeEvent(QResizeEvent* event)
{
	QTableView::resizeEvent(event);

	// Make sure selected items stay visible after resize
	auto selectedRows = this->selectionModel()->selectedRows();
	if (selectedRows.isEmpty() == false)
	{
		this->scrollTo(selectedRows.at(0));
	}
}

void ReorderableTableView::rowsAboutToMoveEventHandler(const QModelIndex&, int sourceStart, int sourceEnd, const QModelIndex&, int)
{
	this->closePersistentEditors(sourceStart, sourceEnd);
}

void ReorderableTableView::rowsMovedEventHandler(const QModelIndex&, int sourceStart, int sourceEnd, const QModelIndex&, int destinationRow)
{
	int rowCount = sourceEnd - sourceStart + 1;
	int firstRow;
	if (destinationRow > sourceStart)
	{
		// When moving down, Qt uses the *following* row as the destination...
		firstRow = destinationRow - rowCount;
	}
	else
	{
		firstRow = destinationRow;
	}
	int lastRow = firstRow + rowCount;

	this->openPersistentEditors(firstRow, lastRow);

	this->updateSelectionFlags();
}

QList<int> ReorderableTableView::getSelectedRowsRanks() const
{
	const auto rows = this->selectionModel()->selectedRows();

	QList<int> selectedRanks;
	for (QModelIndex index : rows)
	{
		selectedRanks.append(index.row());
	}

	return selectedRanks;
}

QList<QPair<int, int> > ReorderableTableView::getSelectedRanksBlocks() const
{
	QList<QPair<int, int>> list;

	auto selectedRows = this->getSelectedRowsRanks();
	auto rowsCount = this->model()->rowCount();

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

void ReorderableTableView::updateSelectionFlags()
{
	this->canSelectionBeRaised  = false;
	this->canSelectionBeLowered = false;
	this->canSelectionBeDeleted = false;

	auto selectedRowsBlocks = this->getSelectedRanksBlocks();
	if (selectedRowsBlocks.count() != 0)
	{
		this->canSelectionBeDeleted = true;

		if (selectedRowsBlocks.count() > 1)
		{
			// Discontinuous selection can always be moved up or down
			this->canSelectionBeRaised  = true;
			this->canSelectionBeLowered = true;
		}
		else
		{
			// If single group, check if at top or at bottom
			if (selectedRowsBlocks.at(0).first != 0)
			{
				this->canSelectionBeRaised  = true;
			}

			if (selectedRowsBlocks.last().first + selectedRowsBlocks.last().second < this->model()->rowCount())
			{
				this->canSelectionBeLowered = true;
			}
		}
	}

	emit this->selectionFlagsChangedEvent();
}
