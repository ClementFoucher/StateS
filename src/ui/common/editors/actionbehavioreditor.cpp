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

// Current class header
#include "actionbehavioreditor.h"


void ActionBehaviorEditor::fillList(uint allowedBehaviors, ActionOnVariable::Behavior_t currentBehavior)
{
	// List allowed behaviors
	if ((allowedBehaviors & static_cast<uint>(ActionOnVariable::Behavior_t::pulse)) != 0 )
	{
		this->addItem(ActionOnVariable::getBehaviorIcon(ActionOnVariable::Behavior_t::pulse),
		              ActionOnVariable::getBehaviorText(ActionOnVariable::Behavior_t::pulse));
	}
	if ((allowedBehaviors & static_cast<uint>(ActionOnVariable::Behavior_t::continuous)) != 0 )
	{
		this->addItem(ActionOnVariable::getBehaviorIcon(ActionOnVariable::Behavior_t::continuous),
		              ActionOnVariable::getBehaviorText(ActionOnVariable::Behavior_t::continuous));
	}
	if ((allowedBehaviors & static_cast<uint>(ActionOnVariable::Behavior_t::assign)) != 0 )
	{
		this->addItem(ActionOnVariable::getBehaviorIcon(ActionOnVariable::Behavior_t::assign),
		              ActionOnVariable::getBehaviorText(ActionOnVariable::Behavior_t::assign));
	}

	// Select current action in list
	this->setCurrentText(ActionOnVariable::getBehaviorText(currentBehavior));

	connect(this, &QComboBox::currentIndexChanged, this, &ActionBehaviorEditor::processIndexChanged);
}

ActionOnVariable::Behavior_t ActionBehaviorEditor::getSelectedBehavior() const
{
	auto currentText = this->currentText();

	if (currentText == ActionOnVariable::getBehaviorText(ActionOnVariable::Behavior_t::pulse))
	{
		return ActionOnVariable::Behavior_t::pulse;
	}
	else if (currentText == ActionOnVariable::getBehaviorText(ActionOnVariable::Behavior_t::continuous))
	{
		return ActionOnVariable::Behavior_t::continuous;
	}
	else if (currentText == ActionOnVariable::getBehaviorText(ActionOnVariable::Behavior_t::assign))
	{
		return ActionOnVariable::Behavior_t::assign;
	}

	// Default value, should not happen unless the list is empty
	return ActionOnVariable::Behavior_t::none;
}

void ActionBehaviorEditor::processIndexChanged(int)
{
	emit this->behaviorChangedEvent();
}
