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

// Parent
#include "statesdialog.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QLabel;

// StateS classes
#include <statestypes.h>
class Equation;
class RangeEditor;


class RangeEditorDialog : public StatesDialog
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	RangeEditorDialog(componentId_t variableId, int rangeL, int rangeR, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	int getRangeL() const;
	int getRangeR() const;

private slots:
	void extractSingleBitSelectedEventHandler(bool checked);
	void extractRangeSelectedEventHandler(bool checked);

	/////
	// Object variables
private:
	shared_ptr<Equation> equation;

	QLabel* title = nullptr;
	RangeEditor* rangeExtractor = nullptr;

};

#endif // RANGEEDITORDIALOG_H
