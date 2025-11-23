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

// Current class header
#include "actiontypeeditor.h"

// StateS classes
#include "statestypes.h"
#include "actiononvariable.h"


void ActionTypeEditor::fillActionList(uint allowedActionTypes, ActionOnVariableType_t currentActionType)
{
	// List allowed actions
	if ((allowedActionTypes & (uint)ActionOnVariableType_t::pulse) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t::pulse),
		              ActionOnVariable::getActionTypeText(ActionOnVariableType_t::pulse));
	}
	if ((allowedActionTypes & (uint)ActionOnVariableType_t::continuous) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t::continuous),
		              ActionOnVariable::getActionTypeText(ActionOnVariableType_t::continuous));
	}
	if ((allowedActionTypes & (uint)ActionOnVariableType_t::set) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t::set),
		              ActionOnVariable::getActionTypeText(ActionOnVariableType_t::set));
	}
	if ((allowedActionTypes & (uint)ActionOnVariableType_t::reset) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t::reset),
		              ActionOnVariable::getActionTypeText(ActionOnVariableType_t::reset));
	}
	if ((allowedActionTypes & (uint)ActionOnVariableType_t::assign) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t::assign),
		              ActionOnVariable::getActionTypeText(ActionOnVariableType_t::assign));
	}
	if ((allowedActionTypes & (uint)ActionOnVariableType_t::increment) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t::increment),
		              ActionOnVariable::getActionTypeText(ActionOnVariableType_t::increment));
	}
	if ((allowedActionTypes & (uint)ActionOnVariableType_t::decrement) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t::decrement),
		              ActionOnVariable::getActionTypeText(ActionOnVariableType_t::decrement));
	}

	// Select current action in list
	this->setCurrentText(ActionOnVariable::getActionTypeText(currentActionType));

	connect(this, &QComboBox::currentIndexChanged, this, &ActionTypeEditor::processIndexChanged);
}

ActionOnVariableType_t ActionTypeEditor::getActionType() const
{
	auto currentText = this->currentText();

	if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariableType_t::pulse))
	{
		return ActionOnVariableType_t::pulse;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariableType_t::continuous))
	{
		return ActionOnVariableType_t::continuous;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariableType_t::set))
	{
		return ActionOnVariableType_t::set;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariableType_t::reset))
	{
		return ActionOnVariableType_t::reset;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariableType_t::assign))
	{
		return ActionOnVariableType_t::assign;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariableType_t::increment))
	{
		return ActionOnVariableType_t::increment;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariableType_t::decrement))
	{
		return ActionOnVariableType_t::decrement;
	}

	// Default value, should not happen unless the list is empty
	return ActionOnVariableType_t::none;
}

void ActionTypeEditor::processIndexChanged(int)
{
	emit this->actionTypeChangedEvent(this);
}
