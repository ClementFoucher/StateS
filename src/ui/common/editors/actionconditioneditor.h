/*
 * Copyright © 2026 Clément Foucher
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

#ifndef ACTIONCONDITIONEDITOR_H
#define ACTIONCONDITIONEDITOR_H

// Parent
#include "cellwitheditandclearbuttonseditor.h"

// Qt
class QLabel;

// StateS
class Equation;
class EquationEditorDialog;


class ActionConditionEditor : public CellWithEditAndClearButtonsEditor
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionConditionEditor(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setCondition(std::shared_ptr<const Equation> condition);
	std::shared_ptr<Equation> getCondition() const;

private slots:
	virtual void editButtonClickedEventHandler()  override;
	virtual void clearButtonClickedEventHandler() override;

	void equationEditorDialogClosedEventHandler(int result);

private:
	virtual bool shouldClearButtonBeVisibile() override;

	void updateDisplay();

	/////
	// Object variables
private:
	std::shared_ptr<Equation> condition;

	QLabel* conditionText = nullptr;

	EquationEditorDialog* equationEditorDialog = nullptr;

};

#endif // ACTIONCONDITIONEDITOR_H
