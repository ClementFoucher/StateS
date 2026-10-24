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

#ifndef VARIABLETABLEVIEW_H
#define VARIABLETABLEVIEW_H

// Parent
#include "reorderabletableview.h"

// StateS
#include "machine.h"
#include "variabletablemodel.h"


class VariableTableView : public ReorderableTableView
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class ContextAction_t : int
	{
		cancel,
		deleteVar,
		raise,
		lower,
		rename,
		changeValue,
		resizeBitVector
	};

	/////
	// Constructors/destructors
public:
	explicit VariableTableView(Machine::VariableNature_t tableNature, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void initialize();

protected slots:
	virtual void rowsInserted(const QModelIndex& parent, int start, int end) override;

protected:
	virtual void contextMenuEvent(QContextMenuEvent* event) override;

	virtual void openPersistentEditors(int firstRow = -1, int firstColumn = -1, int lastRow = -1, int lastColumn = -1) override;

private slots:
	void processMenuEventHandler(QAction* action);
	void refreshPersistentEditorsEventHandler();

	/////
	// Object variables
private:
	VariableTableModel* tableModel = nullptr;
	QMap<VariableTableModel::ColumnRole_t, int> columnsRoles;

	int currentMenuRow = -1;

};

#endif // VARIABLETABLEVIEW_H
