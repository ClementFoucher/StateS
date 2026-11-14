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

#ifndef ACTIONBEHAVIOREDITOR_H
#define ACTIONBEHAVIOREDITOR_H

// Parent
#include "discreetcombobox.h"

// StateS
#include "actiononvariable.h"


/**
 * @brief The ActionBehaviorEditor class displays
 * a drop-down list of possible action behaviors.
 */
class ActionBehaviorEditor : public DiscreetComboBox
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionBehaviorEditor(QWidget* parent = nullptr) : DiscreetComboBox(parent) {}

	/////
	// Object functions
public:
	void fillList(uint allowedBehaviors, ActionOnVariable::Behavior_t currentBehavior);
	ActionOnVariable::Behavior_t getSelectedBehavior() const;

private slots:
	void processIndexChanged(int);

	/////
	// Signals
signals:
	void behaviorChangedEvent();

};

#endif // ACTIONBEHAVIOREDITOR_H
