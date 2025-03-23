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

#ifndef EQUATIONEDITORDIALOG_H
#define EQUATIONEDITORDIALOG_H

// Parent
#include <QDialog>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QWidget;

// StateS classes
class EquationEditorWidget;
class Equation;


class EquationEditorDialog : public QDialog
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit EquationEditorDialog(shared_ptr<const Equation> initialEquation, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	shared_ptr<Equation> getResultEquation() const;

protected:
	virtual void keyPressEvent  (QKeyEvent*   event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;

	/////
	// Object variables
private:
	EquationEditorWidget* equationDisplay = nullptr;

};

#endif // EQUATIONEDITORDIALOG_H

