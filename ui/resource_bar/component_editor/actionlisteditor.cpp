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
#include "actionlisteditor.h"

// StateS classes
#include "svgimagegenerator.h"
#include "machineactuatorcomponent.h"
#include "signal.h"


ActionListEditor::ActionListEditor(shared_ptr<MachineActuatorComponent> actuator, shared_ptr<Signal> signal, QWidget* parent) :
    QComboBox(parent)
{
    this->actuator = actuator;
    this->signal = signal;

    uint allowedActionTypes = actuator->getAllowedActionTypes();

    if ((allowedActionTypes  & MachineActuatorComponent::pulse) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/pulse"))), tr("Pulse"));
    if ((allowedActionTypes  & MachineActuatorComponent::activeOnState) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/active_on_state"))), tr("Active on state"));
    if ((allowedActionTypes  & MachineActuatorComponent::set) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/rising_edge"))), tr("Set"));
    if ((allowedActionTypes  & MachineActuatorComponent::reset) != 0 )
        this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/falling_edge"))), tr("Reset"));

    if (signal->getSize() > 1)
    {
        int param1 = actuator->getActionParam1(signal);
        int param2 = actuator->getActionParam2(signal);

        if ( !( (param1 != -1) && (param2 == -1) ) )
        {
            {
                if ((allowedActionTypes  & MachineActuatorComponent::assign) != 0 )
                    this->addItem(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/assign"))), tr("Assign"));
            }
        }
    }

    updateIndex();

    // Nice one... Qt5 style connect is sometime complicated to use
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ActionListEditor::treatIndexChanged);
}

void ActionListEditor::treatIndexChanged(int index)
{
    shared_ptr<MachineActuatorComponent> actuator = this->actuator.lock();
    shared_ptr<Signal> signal = this->signal.lock();

    if ( (signal != nullptr) && (actuator != nullptr) )
    {
        if (this->itemText(index) == tr("Pulse"))
        {
            actuator->setActionType(signal, MachineActuatorComponent::action_types::pulse);
        }
        else if (this->itemText(index) == tr("Active on state"))
        {
            actuator->setActionType(signal, MachineActuatorComponent::action_types::activeOnState);
        }
        else if (this->itemText(index) == tr("Set"))
        {
            actuator->setActionType(signal, MachineActuatorComponent::action_types::set);
        }
        else if (this->itemText(index) == tr("Reset"))
        {
            actuator->setActionType(signal, MachineActuatorComponent::action_types::reset);
        }
        else if (this->itemText(index) == tr("Assign"))
        {
            actuator->setActionType(signal, MachineActuatorComponent::action_types::assign);
        }
    }
    else
    {
        this->clear();
    }
}

void ActionListEditor::updateIndex()
{
    shared_ptr<MachineActuatorComponent> actuator = this->actuator.lock();
    shared_ptr<Signal> signal = this->signal.lock();

    if ( (signal != nullptr) && (actuator != nullptr) )
    {

        MachineActuatorComponent::action_types currentActionType = actuator->getActionType(signal);

        if (currentActionType == MachineActuatorComponent::action_types::pulse)
        {
            this->setCurrentText(tr("Pulse"));
        }
        else if (currentActionType == MachineActuatorComponent::action_types::activeOnState)
        {
            this->setCurrentText(tr("Active on state"));
        }
        else if (currentActionType == MachineActuatorComponent::action_types::set)
        {
            this->setCurrentText(tr("Set"));
        }
        else if (currentActionType == MachineActuatorComponent::action_types::reset)
        {
            this->setCurrentText(tr("Reset"));
        }
        else if (currentActionType == MachineActuatorComponent::action_types::assign)
        {
            this->setCurrentText(tr("Assign"));
        }
    }
    else
    {
        this->clear();
    }
}
