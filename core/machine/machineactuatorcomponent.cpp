/*
 * Copyright © 2014-2015 Clément Foucher
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
#include "machineactuatorcomponent.h"

// StateS classes
#include "signal.h"
#include "machine.h"


MachineActuatorComponent::MachineActuatorComponent(Machine* owningMachine) :
    MachineComponent(owningMachine)
{
}

void MachineActuatorComponent::signalResizedEventHandler()
{
    foreach(Signal* sig, this->actions)
    {
        // If signal is reset, nothig to care about.

        // Assign values must be resized

        // If signal was size one and is now more or conversly,
        // we must change action type.

        if (sig->getSize() != 1)
        {
            if (actionType[sig] == action_types::pulse)
            {
                // This signal changed size (was size 1)
                actionType[sig] = action_types::assign;
                actionValue[sig] = LogicValue::getValue1(sig->getSize());
            }
            else if (actionType[sig] == action_types::set)
            {
                // This signal changed size (was size 1)
                actionType[sig] = action_types::assign;
                actionValue[sig] = LogicValue::getValue1(sig->getSize());
            }
            else if (actionType[sig] == action_types::assign)
            {
                if (actionValue[sig].getSize() != sig->getSize())
                {
                    // This signal changed size
                    actionValue[sig].resize(sig->getSize());
                }
            }
        }
        else // Signal size is 1
        {
            if (actionType[sig] == action_types::assign)
            {
                // This signal changed size (was size > 1)
                actionType[sig] = action_types::set;
                actionValue.remove(sig);
            }
        }
    }

    emit elementConfigurationChangedEvent();
}

QList<Signal*> MachineActuatorComponent::getActions() const
{
    return actions;
}

void MachineActuatorComponent::setActions(const QList<Signal*>& newActions)
{
    foreach(Signal* sig, newActions)
    {
        addAction(sig);
    }
}


void MachineActuatorComponent::clearActions()
{
    QList<Signal*> actionsToDelete = actions;

    foreach(Signal* sig, actionsToDelete)
    {
        removeAction(sig);
    }

    actionType.clear();

    emit elementConfigurationChangedEvent();
}

void MachineActuatorComponent::addAction(Signal* signal)
{
    actions.append(signal);
    if (signal->getSize() == 1)
        actionType[signal] = action_types::pulse;
    else
        actionType[signal] = action_types::reset;

    // Handle these events
    connect(signal, &Signal::signalDeletedEvent,       this, &MachineActuatorComponent::removeAction);
    connect(signal, &Signal::signalResizedEvent, this, &MachineActuatorComponent::signalResizedEventHandler);
    // Propagate these events
    connect(signal, &Signal::signalConfigurationChangedEvent, this, &MachineActuatorComponent::elementConfigurationChangedEvent);
    connect(signal, &Signal::signalStateChangedEvent,               this, &MachineActuatorComponent::elementStateChangedEvent);

    emit elementConfigurationChangedEvent();
}

void MachineActuatorComponent::addActionByName(const QString& signalName)
{
    foreach (Signal* var, getOwningMachine()->getWrittableSignals())
    {
        if (var->getName() == signalName)
        {
            addAction(var);
            break;
        }
    }
}

void MachineActuatorComponent::removeAction(Signal* signal)
{
    disconnect(signal, &Signal::signalDeletedEvent,                    this, &MachineActuatorComponent::removeAction);
    disconnect(signal, &Signal::signalResizedEvent,              this, &MachineActuatorComponent::signalResizedEventHandler);
    disconnect(signal, &Signal::signalConfigurationChangedEvent, this, &MachineActuatorComponent::elementConfigurationChangedEvent);
    disconnect(signal, &Signal::signalStateChangedEvent,               this, &MachineActuatorComponent::elementStateChangedEvent);

    actions.removeAll(signal);

    actionType. remove(signal);
    actionValue.remove(signal);

    emit elementConfigurationChangedEvent();
}


bool MachineActuatorComponent::removeActionByName(const QString& signalName)
{
    foreach (Signal* sig, actions)
    {
        if (sig->getName() == signalName)
        {
            removeAction(sig);
            return true;
        }
    }

    return false;
}

void MachineActuatorComponent::activateActions()
{
    foreach (Signal* sig, actions)
    {
        if (actionType[sig] == action_types::pulse)
        {
            sig->set();
        }
        else if (actionType[sig] == action_types::set)
        {
            if (sig->getSize() == 1)
                sig->set();
            else
                sig->setCurrentValue(actionValue[sig]);
        }
        else if (actionType[sig] == action_types::reset)
        {
            sig->resetValue();
        }
        else if (actionType[sig] == action_types::assign)
        {
            sig->setCurrentValue(actionValue[sig]);
        }
    }
}

void MachineActuatorComponent::setActionType(Signal* signal, action_types type)
{
    actionType[signal] = type;

    if ( (type == action_types::pulse) ||
         (type == action_types::reset) ||
         (type == action_types::set)
         )
    {
        if (actionValue.contains(signal))
            actionValue.remove(signal);
    }
    else
    {
        if (!actionValue.contains(signal))
            actionValue[signal] = LogicValue::getValue0(signal->getSize());
    }

    emit elementConfigurationChangedEvent();
}

bool MachineActuatorComponent::setActionValue(Signal* signal, LogicValue value)
{
    if (signal->getSize() == value.getSize())
    {
        actionValue[signal] = value;
        emit elementConfigurationChangedEvent();

        return true;
    }
    else
        return false;
}


MachineActuatorComponent::action_types MachineActuatorComponent::getActionType(Signal* variable)
{
    return actionType[variable];
}

LogicValue MachineActuatorComponent::getActionValue(Signal *variable)
{
    if (actionValue.contains(variable))
        return actionValue[variable];
    else
        return LogicValue::getNullValue();
}
