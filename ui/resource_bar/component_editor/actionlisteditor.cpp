/*
 * Copyright © 2014 Clément Foucher
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

#include "actionlisteditor.h"

#include "machineactuatorcomponent.h"
#include "signal.h"

ActionListEditor::ActionListEditor(MachineActuatorComponent* actuator, Signal* signal, QWidget* parent) :
    QComboBox(parent)
{
    this->actuator = actuator;
    this->signal = signal;

    if (signal->getSize() == 1)
    {
        this->addItem(tr("Pulse"));
        this->addItem(tr("Set"));
        this->addItem(tr("Reset"));
    }
    else
    {
        this->addItem(tr("Assign"));
        this->addItem(tr("Reset"));
    }

    updateIndex();

    // Nice one... Qt5 style connect is sometime complicated to use
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ActionListEditor::treatIndexChanged);
}

void ActionListEditor::treatIndexChanged(int index)
{
    if (signal->getSize() == 1)
    {
        if (index == 0)
        {
            actuator->setActionType(this->signal, MachineActuatorComponent::action_types::pulse);
        }
        else if (index == 1)
        {
            actuator->setActionType(this->signal, MachineActuatorComponent::action_types::set);
        }
        else if (index == 2)
        {
            actuator->setActionType(this->signal, MachineActuatorComponent::action_types::reset);
        }
    }
    else
    {
        if (index == 0)
        {
            actuator->setActionType(this->signal, MachineActuatorComponent::action_types::assign);
        }
        else if (index == 1)
        {
            actuator->setActionType(this->signal, MachineActuatorComponent::action_types::reset);
        }
    }
}

void ActionListEditor::updateIndex()
{
    MachineActuatorComponent::action_types currentActionType = actuator->getActionType(signal);

    if (signal->getSize() == 1)
    {
        if (currentActionType == MachineActuatorComponent::action_types::pulse)
        {
            this->setCurrentIndex(0);
        }
        else if (currentActionType == MachineActuatorComponent::action_types::set)
        {
            this->setCurrentIndex(1);
        }
        else if (currentActionType == MachineActuatorComponent::action_types::reset)
        {
            this->setCurrentIndex(2);
        }
    }
    else
    {
        if (currentActionType == MachineActuatorComponent::action_types::assign)
        {
            this->setCurrentIndex(0);
        }
        else if (currentActionType == MachineActuatorComponent::action_types::reset)
        {
            this->setCurrentIndex(1);
        }
    }
}
