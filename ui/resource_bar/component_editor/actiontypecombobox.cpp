/*
 * Copyright © 2014-2016 Clément Foucher
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
#include "svgimagegenerator.h"
#include "actiononsignal.h"
#include "machineactuatorcomponent.h"
#include "statesexception.h"


ActionTypeComboBox::ActionTypeComboBox(uint allowedActionTypes, shared_ptr<ActionOnSignal> action, QWidget* parent) :
    QComboBox(parent)
{
    this->action = action;

    // List allowed actions
    if ((allowedActionTypes & MachineActuatorComponent::pulse) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/pulse"))), tr("Pulse"));
    if ((allowedActionTypes & MachineActuatorComponent::activeOnState) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/active_on_state"))), tr("Active on state"));
    if ((allowedActionTypes & MachineActuatorComponent::set) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/rising_edge"))), tr("Set"));
    if ((allowedActionTypes & MachineActuatorComponent::reset) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/falling_edge"))), tr("Reset"));

    if (action->getActionSize() > 1)
    {
        if ((allowedActionTypes & MachineActuatorComponent::assign) != 0 )
            this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/assign"))), tr("Assign"));
    }

    // Select current action in list
    switch(action->getActionType())
    {
    case ActionOnSignal::action_types::pulse:
        this->setCurrentText(tr("Pulse"));
        break;
    case ActionOnSignal::action_types::activeOnState:
        this->setCurrentText(tr("Active on state"));
        break;
    case ActionOnSignal::action_types::set:
        this->setCurrentText(tr("Set"));
        break;
    case ActionOnSignal::action_types::reset:
        this->setCurrentText(tr("Reset"));
        break;
    case ActionOnSignal::action_types::assign:
        this->setCurrentText(tr("Assign"));
        break;
    }

    // Nice one... Qt5 style connect is sometime complicated to use
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ActionTypeComboBox::treatIndexChanged);
}

void ActionTypeComboBox::treatIndexChanged(int index)
{
    shared_ptr<ActionOnSignal> l_action = this->action.lock();

    if (l_action != nullptr)
    {
        try
        {
            if (this->itemText(index) == tr("Pulse"))
            {
                l_action->setActionType(ActionOnSignal::action_types::pulse); // Throws StatesException
            }
            else if (this->itemText(index) == tr("Active on state"))
            {
                l_action->setActionType(ActionOnSignal::action_types::activeOnState); // Throws StatesException
            }
            else if (this->itemText(index) == tr("Set"))
            {
                l_action->setActionType(ActionOnSignal::action_types::set); // Throws StatesException
            }
            else if (this->itemText(index) == tr("Reset"))
            {
                l_action->setActionType(ActionOnSignal::action_types::reset); // Throws StatesException
            }
            else if (this->itemText(index) == tr("Assign"))
            {
                l_action->setActionType(ActionOnSignal::action_types::assign); // Throws StatesException
            }
        }
        catch (const StatesException& e)
        {
            if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_type) )
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
