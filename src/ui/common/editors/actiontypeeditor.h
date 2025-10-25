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

#ifndef ACTIONTYPEEDITOR_H
#define ACTIONTYPEEDITOR_H

// Parent
#include <QComboBox>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class ActionOnVariable;


/**
 * @brief The ActionTypeEditor class displays
 * a drop-down list displaying the allowed action
 * types for a specific action, and changes the
 * action type according to user selection in the
 * list.
 */
class ActionTypeEditor : public QComboBox
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionTypeEditor(uint allowedActionTypes, shared_ptr<ActionOnVariable> action, QWidget* parent = nullptr);

	/////
	// Object functions
private slots:
	void processIndexChanged(int index);

	/////
	// Object variables
private:
	weak_ptr<ActionOnVariable> action;

};

#endif // ACTIONTYPEEDITOR_H
