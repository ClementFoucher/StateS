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

#ifndef ACTIONTABLEVIEW_H
#define ACTIONTABLEVIEW_H

// Parent class
#include "reorderabletableview.h"

// C++ classes
#include <memory>
using namespace std;

// StateS Classes
#include "statestypes.h"
class ActionTableModel;
class RangeEditorDialog;
class ActionOnVariable;


class ActionTableView : public ReorderableTableView
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum ContextAction
	{
		Cancel             = 0,
		DeleteAction       = 1,
		AffectSwitchWhole  = 2,
		AffectSwitchSingle = 3,
		AffectSwitchRange  = 4,
		AffectEditRange    = 5,
		EditValue          = 6,
		MoveUp             = 7,
		MoveDown           = 8
	};

	/////
	// Constructors/destructors
public:
	explicit ActionTableView(componentId_t actuatorId, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void initialize();
	void addAction(const QString& variableName);

protected:
	virtual void keyPressEvent   (QKeyEvent*         ev) override;
	virtual void contextMenuEvent(QContextMenuEvent* ev) override;

private slots:
	void processContextMenuEventHandler(QAction* action);
	void rangeEditorClosedEventHandler(int result);

private:
	virtual void openPersistentEditors (int firstRow = -1, int lastRow = -1) override;
	virtual void closePersistentEditors(int firstRow = -1, int lastRow = -1) override;

	/////
	// Object variables
private:
	componentId_t actuatorId = nullId;

	ActionTableModel* tableModel = nullptr;

	shared_ptr<ActionOnVariable> actionBeingEdited;
	RangeEditorDialog* rangeEditorDialog = nullptr;
	int currentMenuRow = -1;

};

#endif // ACTIONTABLEVIEW_H
