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

#ifndef EQUATIONEDITORCELL_H
#define EQUATIONEDITORCELL_H

// Parent
#include <QWidget>

// Qt
class QHBoxLayout;
class QLabel;
class QPushButton;

// StateS
class Equation;
class EquationEditorDialog;


class EquationEditorCell : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit EquationEditorCell(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setEquation(std::shared_ptr<const Equation> equation);
	std::shared_ptr<Equation> getEquation() const;

	void setIsConditionEditor(bool isConditionEditor);

protected:
	void resizeEvent(QResizeEvent* event) override;

private slots:
	void editEquationButtonPressedEventHandler();
	void clearEquationButtonPressedEventHandler();

	void equationEditorDialogClosedEventHandler(int result);

private:
	void updateDisplay();

	/////
	// Signals
signals:
	void equationChangedEvent(EquationEditorCell* me);

	/////
	// Object variables
private:
	std::shared_ptr<Equation> equation;
	bool isConditionEditor = false;

	QHBoxLayout* mainLayout;

	QLabel* equationText = nullptr;

	QPushButton* buttonEditEquation  = nullptr;
	QPushButton* buttonClearEquation = nullptr;

	EquationEditorDialog* equationEditorDialog = nullptr;

};

#endif // EQUATIONEDITORCELL_H
