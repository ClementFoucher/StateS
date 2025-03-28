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

#ifndef RANGEEDITORDIALOG_H
#define RANGEEDITORDIALOG_H

// Base class
#include <QDialog>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Equation;
class ActionOnVariable;


class RangeEditorDialog : public QDialog
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	RangeEditorDialog(shared_ptr<ActionOnVariable> action, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	int getRangeL() const;
	int getRangeR() const;

	shared_ptr<ActionOnVariable> getAction() const;

	/////
	// Object variables
private:
	shared_ptr<Equation> equation;

	shared_ptr<ActionOnVariable> action;

};

#endif // RANGEEDITORDIALOG_H
