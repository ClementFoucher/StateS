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

#ifndef VARIABLETABLEVIEW_H
#define VARIABLETABLEVIEW_H

// Parent
#include <QTableView>

// StateS classes
#include "statestypes.h"
class VariableTableModel;


class VariableTableView : public QTableView
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class ContextAction_t : int
	{
		cancel          = 0,
		deleteVar       = 1,
		raise           = 2,
		lower           = 3,
		rename          = 4,
		changeValue     = 5,
		resizeBitVector = 6
	};

	enum class ColumnRole
	{
		name,
		size,
		value
	};

	/////
	// Constructors/destructors
public:
	explicit VariableTableView(VariableNature_t tableNature, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void addNewVariable();
	void deleteSelectedVariables();
	void raiseSelectedVariables();
	void lowerSelectedVariables();

	bool getSelectedVariablesCanBeDeleted();
	bool getSelectedVariablesCanBeRaised();
	bool getSelectedVariablesCanBeLowered();

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void resizeEvent     (QResizeEvent* event)      override;

private:
	QList<int>             getSelectedRowsRanks()   const;
	QList<QPair<int, int>> getSelectedRanksBlocks() const;

	void updateSelectionFlags();

protected slots:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)                              override;
	virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles = QList<int>()) override;
	virtual void rowsInserted(const QModelIndex& parent, int start, int end)                                                     override;

private slots:
	void rowsMovedEventHandler();
	void processMenuEventHandler(QAction* action);

	/////
	// Signals
signals:
	void variablesSelectionChangedEvent();

	/////
	// Object variables
private:
	VariableTableModel* tableModel = nullptr;
	QMap<ColumnRole, int> columnsRoles;

	// Flags depending on selection
	bool canSelectedVariablesBeDeleted = false;
	bool canSelectedVariablesBeRaised  = false;
	bool canSelectedVariablesBeLowered = false;

};

#endif // VARIABLETABLEVIEW_H
