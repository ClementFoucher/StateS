/*
 * Copyright © 2014-2025 Clément Foucher
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
 * along with StateS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QPushButton;
class QTableView;
class QItemSelection;

// StateS Classes
#include "statestypes.h"
class CollapsibleWidgetWithTitle;
class ActionOnVariable;
class ActionTableDelegate;


/**
 * @brief The ActionEditor class displays a list of actions
 * for a MachineActuatorComponent and tools to edit it.
 *
 * It has a weak dependence on the actuator, and becomes
 * passive (all functions inhibited) if the actuator is
 * deleted.
 */
class ActionEditor : public QWidget
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
	// Static variables
protected:
	// Remember user choise regarding hint collapse
	static bool hintCollapsed;

	/////
	// Constructors/destructors
public:
	explicit ActionEditor(componentId_t actuatorId, QString title, QWidget* parent = nullptr);

	~ActionEditor();

	/////
	// Object functions
protected:
	virtual void keyPressEvent   (QKeyEvent* e)             override;
	virtual void keyReleaseEvent (QKeyEvent* e)             override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
	void selectionChangedEventHandler(const QItemSelection&, const QItemSelection&);

	void displayAddActionMenu() const;
	void removeSelectedActions();

	void processAddActionMenuEventHandler(QAction* action);
	void processContextMenuEventHandler  (QAction* action);

	void moveSelectedActionsUp();
	void moveSelectedActionsDown();

	void tableChangedEventHandler();

private:
	void fillFirstColumn();
	void updateButtonsEnableState();

	void sortSelectionList();
	void restoreSelection();

	/////
	// Object variables
private:
	componentId_t actuatorId;

	QTableView*                 actionTable        = nullptr;
	QPushButton*                buttonAddAction    = nullptr;
	QPushButton*                buttonRemoveAction = nullptr;
	QPushButton*                buttonMoveUp       = nullptr;
	QPushButton*                buttonMoveDown     = nullptr;
	CollapsibleWidgetWithTitle* hintDisplay        = nullptr;
	ActionTableDelegate*        valueColDelegate   = nullptr;

	QList<weak_ptr<ActionOnVariable>> latestSelection;

};

#endif // ACTIONEDITOR_H
