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

#ifndef VARIABLELISTEDITOR_H
#define VARIABLELISTEDITOR_H

// Parent
#include <QWidget>

// Qt classes
class QPushButton;

// StateS classes
#include "statestypes.h"
class VariableTableView;


class VariableListEditor : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit VariableListEditor(VariableNature_t editorNature, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void keyPressEvent  (QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

private slots:
	void updateButtonsEnableState();

	void buttonAddPressedEventHandler();
	void buttonRemovePressedEventHandler();
	void buttonUpPressedEventHandler();
	void buttonDownPressedEventHandler();

	/////
	// Object variables
private:
	VariableNature_t editorNature;

	VariableTableView* tableView = nullptr;

	QPushButton* buttonAdd    = nullptr;
	QPushButton* buttonRemove = nullptr;
	QPushButton* buttonUp     = nullptr;
	QPushButton* buttonDown   = nullptr;

};

#endif // VARIABLELISTEDITOR_H
