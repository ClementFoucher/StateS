/*
 * Copyright © 2014-2023 Clément Foucher
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
#include "actiontypecombobox.h"

// Debug
#include <QDebug>

// StateS classes
#include "statestypes.h"
#include "pixmapgenerator.h"
#include "actiononsignal.h"
#include "statesexception.h"
#include "exceptiontypes.h"


ActionTypeComboBox::ActionTypeComboBox(uint allowedActionTypes, shared_ptr<ActionOnSignal> action, QWidget* parent) :
    QComboBox(parent)
{
	this->action = action;

	// List allowed actions
	if ((allowedActionTypes & (uint)actuatorAllowedActionType_t::pulse) != 0 )
		this->addItem(QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/pulse"))), tr("Pulse"));
	if ((allowedActionTypes & (uint)actuatorAllowedActionType_t::activeOnState) != 0 )
		this->addItem(QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/active_on_state"))), tr("Active on state"));
	if ((allowedActionTypes & (uint)actuatorAllowedActionType_t::set) != 0 )
		this->addItem(QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/rising_edge"))), tr("Set"));
	if ((allowedActionTypes & (uint)actuatorAllowedActionType_t::reset) != 0 )
		this->addItem(QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/falling_edge"))), tr("Reset"));

	if (action->getActionSize() > 1)
	{
		if ((allowedActionTypes & (uint)actuatorAllowedActionType_t::assign) != 0 )
			this->addItem(QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/assign"))), tr("Assign"));
	}

	// Select current action in list
	switch(action->getActionType())
	{
	case ActionOnSignalType_t::pulse:
		this->setCurrentText(tr("Pulse"));
		break;
	case ActionOnSignalType_t::activeOnState:
		this->setCurrentText(tr("Active on state"));
		break;
	case ActionOnSignalType_t::set:
		this->setCurrentText(tr("Set"));
		break;
	case ActionOnSignalType_t::reset:
		this->setCurrentText(tr("Reset"));
		break;
	case ActionOnSignalType_t::assign:
		this->setCurrentText(tr("Assign"));
		break;
	}

	// Nice one... Qt5 style connect is sometime complicated to use
	connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ActionTypeComboBox::processIndexChanged);
}

void ActionTypeComboBox::processIndexChanged(int index)
{
	shared_ptr<ActionOnSignal> l_action = this->action.lock();

	if (l_action != nullptr)
	{
		try
		{
			if (this->itemText(index) == tr("Pulse"))
			{
				l_action->setActionType(ActionOnSignalType_t::pulse); // Throws StatesException
			}
			else if (this->itemText(index) == tr("Active on state"))
			{
				l_action->setActionType(ActionOnSignalType_t::activeOnState); // Throws StatesException
			}
			else if (this->itemText(index) == tr("Set"))
			{
				l_action->setActionType(ActionOnSignalType_t::set); // Throws StatesException
			}
			else if (this->itemText(index) == tr("Reset"))
			{
				l_action->setActionType(ActionOnSignalType_t::reset); // Throws StatesException
			}
			else if (this->itemText(index) == tr("Assign"))
			{
				l_action->setActionType(ActionOnSignalType_t::assign); // Throws StatesException
			}
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignalError_t::illegal_type) )
			{
				qDebug() << "(ActionTypeComboBox:) Warning! An illegal action type was selected. Type change ignored.";
			}
			else
				throw;
		}
	}
	else
	{
		this->clear();
	}
}
