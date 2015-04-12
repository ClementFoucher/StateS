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


MachineActuatorComponent::MachineActuatorComponent(shared_ptr<Machine> owningMachine) :
    MachineComponent(owningMachine)
{
    // Propagates local events to the more general "configuration changed" event
    connect(this, &MachineActuatorComponent::actionListChangedEvent, this, &MachineComponent::componentStaticConfigurationChangedEvent);
}

void MachineActuatorComponent::signalResizedEventHandler()
{
    this->cleanActionList();

    bool listChanged = false;
    foreach(weak_ptr<Signal> s, this->actions)
    {
        // Assign values must be resized

        // If signal was size one and is now more or conversly,
        // we must change action type.

        shared_ptr<Signal> sig = s.lock();

        QString signame = sig->getName();

        if (sig->getSize() != 1)
        {
            if (actionType[signame] == action_types::activeOnState)
            {
                // This signal changed size (was size 1)
                actionType[signame] = action_types::assign;
                actionValue[signame] = LogicValue::getValue1(sig->getSize());

                listChanged = true;
            }
            else if (actionType[signame] == action_types::pulse)
            {
                // This signal changed size (was size 1)
                actionType[signame] = action_types::assign;
                actionValue[signame] = LogicValue::getValue1(sig->getSize());

                listChanged = true;
            }
            else if (actionType[signame] == action_types::set)
            {
                // This signal changed size (was size 1)
                actionType[signame] = action_types::assign;
                actionValue[signame] = LogicValue::getValue1(sig->getSize());

                listChanged = true;
            }
            else if (actionType[signame] == action_types::assign)
            {
                if (actionValue[signame].getSize() != sig->getSize())
                {
                    // This signal changed size
                    actionValue[signame].resize(sig->getSize());

                    listChanged = true;
                }
            }
        }
        else // Signal size is 1
        {
            if (actionType[signame] == action_types::assign)
            {
                // This signal changed size (was size > 1)
                actionType[signame] = action_types::set;
                actionValue.remove(signame);

                listChanged = true;
            }
        }
    }

    if (listChanged) // Must be true, but anyway...
    {
        emit actionListChangedEvent();
    }
}

void MachineActuatorComponent::cleanActionList()
{
    QList<weak_ptr<Signal>> newActionList;

    foreach(weak_ptr<Signal> sig, this->actions)
    {
        if (!sig.expired())
            newActionList.append(sig);
    }

    bool listChanged = false;
    if (newActionList.count() != this->actions.count())
        listChanged = true;

    this->actions = newActionList;

    if (listChanged)
    {
        emit actionListChangedEvent();
    }

    // At this pont, should clean action type and value too,
    // but we would have to iterate over both lists to check wich
    // one remains...
    // As this function is called frequently, we will limit
    // operations here. Lost fragments will not survive save/load anyway.
}

QList<shared_ptr<Signal> > MachineActuatorComponent::getActions()
{
    this->cleanActionList();

    QList<shared_ptr<Signal>> list;

    foreach(weak_ptr<Signal> sig, this->actions)
    {
        list.append(sig.lock());
    }

    return list;
}

void MachineActuatorComponent::setActions(const QList<shared_ptr<Signal> > &newActions)
{
    foreach(shared_ptr<Signal> sig, newActions)
    {
        // cleanActionList() is already done by addAction()
        addAction(sig);
    }
}


void MachineActuatorComponent::clearActions()
{
    this->cleanActionList();

    bool listChanged = false;
    if (this->actions.count() != 0)
        listChanged = true;

    QList<weak_ptr<Signal>> actionsToDelete = this->actions;

    foreach(weak_ptr<Signal> sig, actionsToDelete)
    {
        // Must be done to disconnect Qt signals
        removeAction(sig.lock());
    }

    // In case there were lost fragments (see comment in cleanActionList())
    actionType.clear();
    actionValue.clear();

    if (listChanged)
    {
        emit actionListChangedEvent();
    }
}

void MachineActuatorComponent::addAction(shared_ptr<Signal> signal)
{
    this->cleanActionList();

    this->actions.append(signal);
    if (signal->getSize() == 1)
    {
        if ((this->allowedActionTypes & activeOnState) != 0)
            actionType[signal->getName()] = action_types::activeOnState;
        else
            actionType[signal->getName()] = action_types::pulse;
    }
    else
        actionType[signal->getName()] = action_types::reset;

    // Handle these events
    connect(signal.get(), &Signal::signalDeletedEvent, this, &MachineActuatorComponent::cleanActionList);
    connect(signal.get(), &Signal::signalResizedEvent, this, &MachineActuatorComponent::signalResizedEventHandler);
    // Propagate these events : any change to the signals are considered changes to the component
    connect(signal.get(), &Signal::signalStaticConfigurationChangedEvent, this, &MachineActuatorComponent::componentStaticConfigurationChangedEvent);
    connect(signal.get(), &Signal::signalDynamicStateChangedEvent,        this, &MachineActuatorComponent::componentDynamicStateChangedEvent);

    emit actionListChangedEvent();
}

void MachineActuatorComponent::addActionByName(const QString& signalName)
{
    shared_ptr<Machine> owningMachine = this->getOwningMachine();

    if (owningMachine != nullptr)
    {
        foreach (shared_ptr<Signal> var, owningMachine->getWrittableSignals())
        {
            if (var->getName() == signalName)
            {
                addAction(var);
                break;
            }
        }
    }
}

void MachineActuatorComponent::removeAction(shared_ptr<Signal> signal)
{
    if (signal != nullptr)
    {
        disconnect(signal.get(), &Signal::signalDeletedEvent, this, &MachineActuatorComponent::cleanActionList);
        disconnect(signal.get(), &Signal::signalResizedEvent, this, &MachineActuatorComponent::signalResizedEventHandler);
        disconnect(signal.get(), &Signal::signalStaticConfigurationChangedEvent, this, &MachineActuatorComponent::componentStaticConfigurationChangedEvent);
        disconnect(signal.get(), &Signal::signalDynamicStateChangedEvent,        this, &MachineActuatorComponent::componentDynamicStateChangedEvent);

        foreach(weak_ptr<Signal> s, this->actions)
        {
            if (s.lock() == signal)
            {
                s.reset();
            }
        }

        actionType. remove(signal->getName());
        actionValue.remove(signal->getName());

        this->cleanActionList();

        emit actionListChangedEvent();
    }
}

void MachineActuatorComponent::setAllowedActionTypes(uint flags)
{
    this->allowedActionTypes = flags;
}

uint MachineActuatorComponent::getAllowedActionTypes() const
{
    return this->allowedActionTypes;
}


bool MachineActuatorComponent::removeActionByName(const QString& signalName)
{
    foreach (shared_ptr<Signal> sig, this->getActions())
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
    foreach (shared_ptr<Signal> sig, this->getActions())
    {
        QString signame = sig->getName();

        if (actionType[signame] == action_types::activeOnState)
        {
            sig->set();
        }
        else if (actionType[signame] == action_types::pulse)
        {
            sig->set();
        }
        else if (actionType[signame] == action_types::set)
        {
            if (sig->getSize() == 1)
                sig->set();
            else
                sig->setCurrentValue(actionValue[signame]);
        }
        else if (actionType[signame] == action_types::reset)
        {
            sig->resetValue();
        }
        else if (actionType[signame] == action_types::assign)
        {
            sig->setCurrentValue(actionValue[signame]);
        }
    }
}

void MachineActuatorComponent::setActionType(shared_ptr<Signal> signal, action_types type)
{
    this->cleanActionList();

    QString signame = signal->getName();

    actionType[signame] = type;

    if ( (type == action_types::activeOnState) ||
         (type == action_types::pulse) ||
         (type == action_types::reset) ||
         (type == action_types::set)
         )
    {
        if (actionValue.contains(signame))
            actionValue.remove(signame);
    }
    else
    {
        if (!actionValue.contains(signame))
            actionValue[signame] = LogicValue::getValue0(signal->getSize());
    }

    emit actionListChangedEvent();
}

bool MachineActuatorComponent::setActionValue(shared_ptr<Signal> signal, LogicValue value)
{
    if (signal->getSize() == value.getSize())
    {
        actionValue[signal->getName()] = value;

        emit actionListChangedEvent();

        return true;
    }
    else
        return false;
}


MachineActuatorComponent::action_types MachineActuatorComponent::getActionType(shared_ptr<Signal> variable)
{
    this->cleanActionList();

    return actionType[variable->getName()];
}

LogicValue MachineActuatorComponent::getActionValue(shared_ptr<Signal> variable)
{
    if (actionValue.contains(variable->getName()))
        return actionValue[variable->getName()];
    else
        return LogicValue::getNullValue();
}
