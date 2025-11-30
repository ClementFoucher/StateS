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

#ifndef REORDERABLETABLEVIEW_H
#define REORDERABLETABLEVIEW_H

// Parent class
#include <QTableView>


class ReorderableTableView : public QTableView
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ReorderableTableView(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void addNewRow();
	void deleteSelectedRows();
	void raiseSelectedRows();
	void lowerSelectedRows();

	bool getSelectionCanBeDeleted();
	bool getSelectionCanBeRaised();
	bool getSelectionCanBeLowered();

	virtual void setModel(QAbstractItemModel* model) override;

protected slots:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)                              override;
	virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles = QList<int>()) override;
	virtual void rowsInserted(const QModelIndex& parent, int start, int end)                                                     override;
	virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)                                             override;

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

private slots:
	void rowsAboutToMoveEventHandler(const QModelIndex&, int sourceStart, int sourceEnd, const QModelIndex&, int);
	void rowsMovedEventHandler(const QModelIndex&, int sourceStart, int sourceEnd, const QModelIndex&, int destinationRow);

private:
	QList<int>             getSelectedRowsRanks()   const;
	QList<QPair<int, int>> getSelectedRanksBlocks() const;

	virtual void openPersistentEditors (int firstRow = -1, int lastRow = -1) = 0;
	virtual void closePersistentEditors(int firstRow = -1, int lastRow = -1) = 0;

	void updateSelectionFlags();

	/////
	// Signals
signals:
	void selectionFlagsChangedEvent();

	/////
	// Object variables
private:
	bool canSelectionBeDeleted = false;
	bool canSelectionBeRaised  = false;
	bool canSelectionBeLowered = false;

};

#endif // REORDERABLETABLEVIEW_H
