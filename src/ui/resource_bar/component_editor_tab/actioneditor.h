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

// Qt classes
class QPushButton;

// StateS Classes
#include "statestypes.h"
class HintWidget;
class ActionTableView;


/**
 * @brief The ActionEditor class displays a list of actions
 * for a MachineActuatorComponent and tools to edit it.
 */
class ActionEditor : public QWidget
{
	Q_OBJECT

	/////
	// Static variables
protected:
	// Remember user choice regarding hint collapse
	static bool hintCollapsed;

	/////
	// Constructors/destructors
public:
	explicit ActionEditor(componentId_t actuatorId, QWidget* parent = nullptr);

	~ActionEditor();

	/////
	// Object functions
private slots:
	void buttonAddPushedEventHandler();
	void buttonRemovePushedEventHandler();
	void buttonMoveUpPushedEventHandler();
	void buttonMoveDownPushedEventHandler();

	void processAddActionMenuEventHandler(QAction* action);

	void updateButtonsEnableState();

	/////
	// Object variables
private:
	ActionTableView* actionTable = nullptr;

	QPushButton* buttonAddAction    = nullptr;
	QPushButton* buttonRemoveAction = nullptr;
	QPushButton* buttonMoveUp       = nullptr;
	QPushButton* buttonMoveDown     = nullptr;

	HintWidget* hintDisplay = nullptr;
};

#endif // ACTIONEDITOR_H
