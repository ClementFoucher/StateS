/*
 * Copyright © 2014-2026 Clément Foucher
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
#include "discreetcombobox.h"

// StateS
#include "actiononvariable.h"


/**
 * @brief The ActionTypeEditor class displays
 * a drop-down list of action types.
 */
class ActionTypeEditor : public DiscreetComboBox
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionTypeEditor(QWidget* parent = nullptr) : DiscreetComboBox(parent) {}

	/////
	// Object functions
public:
	void fillActionList(uint allowedActionTypes, ActionOnVariable::Type_t currentActionType);
	ActionOnVariable::Type_t getActionType() const;

private slots:
	void processIndexChanged(int);

	/////
	// Signals
signals:
	void actionTypeChangedEvent(ActionTypeEditor* me);

};

#endif // ACTIONTYPEEDITOR_H
