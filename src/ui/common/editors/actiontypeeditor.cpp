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
#include "actiontypeeditor.h"

// StateS
#include "statestypes.h"


void ActionTypeEditor::fillActionList(uint allowedActionTypes, ActionOnVariable::Type_t currentActionType)
{
	// List allowed actions
	if ((allowedActionTypes & static_cast<uint>(ActionOnVariable::Type_t::pulse)) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t::pulse),
		              ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::pulse));
	}
	if ((allowedActionTypes & static_cast<uint>(ActionOnVariable::Type_t::continuous)) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t::continuous),
		              ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::continuous));
	}
	if ((allowedActionTypes & static_cast<uint>(ActionOnVariable::Type_t::set)) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t::set),
		              ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::set));
	}
	if ((allowedActionTypes & static_cast<uint>(ActionOnVariable::Type_t::reset)) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t::reset),
		              ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::reset));
	}
	if ((allowedActionTypes & static_cast<uint>(ActionOnVariable::Type_t::assign)) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t::assign),
		              ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::assign));
	}
	if ((allowedActionTypes & static_cast<uint>(ActionOnVariable::Type_t::increment)) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t::increment),
		              ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::increment));
	}
	if ((allowedActionTypes & static_cast<uint>(ActionOnVariable::Type_t::decrement)) != 0 )
	{
		this->addItem(ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t::decrement),
		              ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::decrement));
	}

	// Select current action in list
	this->setCurrentText(ActionOnVariable::getActionTypeText(currentActionType));

	connect(this, &QComboBox::currentIndexChanged, this, &ActionTypeEditor::processIndexChanged);
}

ActionOnVariable::Type_t ActionTypeEditor::getActionType() const
{
	auto currentText = this->currentText();

	if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::pulse))
	{
		return ActionOnVariable::Type_t::pulse;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::continuous))
	{
		return ActionOnVariable::Type_t::continuous;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::set))
	{
		return ActionOnVariable::Type_t::set;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::reset))
	{
		return ActionOnVariable::Type_t::reset;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::assign))
	{
		return ActionOnVariable::Type_t::assign;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::increment))
	{
		return ActionOnVariable::Type_t::increment;
	}
	else if (currentText == ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t::decrement))
	{
		return ActionOnVariable::Type_t::decrement;
	}

	// Default value, should not happen unless the list is empty
	return ActionOnVariable::Type_t::none;
}

void ActionTypeEditor::processIndexChanged(int)
{
	emit this->actionTypeChangedEvent(this);
}
