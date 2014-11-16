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

#include "signalactionslist.h"

#include "logicvariable.h"
#include "machineactuatorelement.h"

SignalActionsList::SignalActionsList(MachineActuatorElement* actuator, LogicVariable* signal, QWidget* parent) :
    QComboBox(parent)
{
    this->actuator = actuator;
    this->signal = signal;

    LogicVariable::action_types currentActionType = actuator->getActionType(signal);

    this->addItem(tr("Pulse"));
    this->addItem(tr("Set"));
    this->addItem(tr("Reset"));

    if (currentActionType == LogicVariable::action_types::pulse)
    {
        this->setCurrentIndex(0);
    }
    else if (currentActionType == LogicVariable::action_types::set)
    {
        this->setCurrentIndex(1);
    }
    else if (currentActionType == LogicVariable::action_types::reset)
    {
        this->setCurrentIndex(2);
    }

    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(treatIndexChanged(int)));
}

void SignalActionsList::treatIndexChanged(int index)
{
    if (index == 0)
    {
        actuator->setActionType(this->signal, LogicVariable::action_types::pulse);
    }
    else if (index == 1)
    {
        actuator->setActionType(this->signal, LogicVariable::action_types::set);
    }
    else if (index == 2)
    {
        actuator->setActionType(this->signal, LogicVariable::action_types::reset);
    }
}
