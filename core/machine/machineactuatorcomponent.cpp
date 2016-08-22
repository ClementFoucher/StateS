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

// Qt classes
#include <QDebug>

// StateS classes
#include "StateS_signal.h"
#include "machine.h"
#include "statesexception.h"



MachineActuatorComponent::MachineActuatorComponent(shared_ptr<Machine> owningMachine) :
    MachineComponent(owningMachine)
{
    // Propagates local events to the more general "configuration changed" event
    connect(this, &MachineActuatorComponent::actionListChangedEvent, this, &MachineComponent::componentStaticConfigurationChangedEvent);
}

void MachineActuatorComponent::signalResizedEventHandler(shared_ptr<Signal> emitter)
{
    this->cleanActionList();

    try
    {
        QString signame = emitter->getName();

        if (emitter->getSize() == 1) // We used to be a vector signal (size changed and is now 1 => used to be > 1)
        {
            // Remove parameters
            actionParam1.remove(signame);
            actionParam2.remove(signame);

            // Switch to implicit value whatever we were
            actionValue.remove(signame);

            if (actionType[signame] == action_types::assign) // Assign is illegal for single bit signals
                actionType[signame] = action_types::set;
        }
        else // We are now a vector signal
        {
            if (!actionParam1.contains(signame)) // We used to be a single bit signal
            {
                // Switch to whole range extract
                actionParam1[signame] = -1;
                actionParam2[signame] = -1;

                switch (actionType[signame])
                {
                case action_types::set:
                case action_types::reset:
                    // Value remains implicit
                    break;
                case action_types::activeOnState:
                case action_types::assign:
                case action_types::pulse:
                    // Switch to explicit value
                    actionValue[signame] = LogicValue::getValue1(emitter->getSize());
                    break;
                }
            }
            else // We were vector, we're still vector
            {
                // We have to check if parameters are still correct

                int signalSize = emitter->getSize();

                if ( (actionParam1[signame] == -1) && (actionParam2[signame] == -1) ) // Whole range extract
                {
                    // Just resize value if not impicit
                    if (actionValue.contains(signame))
                        actionValue[signame].resize(signalSize); // Throws StatesException
                }
                else if ( (actionParam1[signame] != -1) && (actionParam2[signame] == -1) ) // Single bit extract
                {
                    // We have to check if parameter is still in range
                    if (actionParam1[signame] >= signalSize)
                        actionParam1[signame] = signalSize-1;

                    // Nothing to do with value, always implicit
                }
                else // Range extract
                {
                    // Check if parameters are still in range
                    if (actionParam1[signame] >= signalSize)
                    {
                        actionParam1[signame] = signalSize-1;

                        if (actionParam2[signame] >= actionParam1[signame])
                        {
                            actionParam2[signame] = actionParam1[signame]-1;
                        }

                        if (actionValue.contains(signame))
                        {
                            // Resize action value
                            signalSize = actionParam1[signame] - actionParam2[signame] + 1;
                            actionValue[signame].resize(signalSize); // Throws StatesException
                        }
                    }
                }
            }
        }

        emit actionListChangedEvent();
    }
    catch (const StatesException& e)
    {
        if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
        {
            qDebug() << "(MachineActuatorComponent:) Error! Unable to resize action value. Action is probably broken now.";
        }
        else
            throw;
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

    // At this point, should clean action type, value and param too,
    // but we would have to iterate over all lists to check which
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
        list.append(sig.lock()); // No need to check for nullness: we just cleaned action list
    }

    return list;
}


void MachineActuatorComponent::clearActions()
{
    this->cleanActionList();

    if (this->actions.count() != 0)
    {
        QList<weak_ptr<Signal>> actionsToDelete = this->actions;

        foreach(weak_ptr<Signal> sig, actionsToDelete)
        {
            // Must be done to disconnect Qt signals
            removeAction(sig.lock()); // No need to check for nullness: we just cleaned action list
        }

        // In case there were lost fragments (see comment in cleanActionList())
        actionType.clear();
        actionValue.clear();
        actionParam1.clear();
        actionParam2.clear();

        emit actionListChangedEvent();
    }
}

void MachineActuatorComponent::addAction(shared_ptr<Signal> signal)
{
    this->cleanActionList();

    this->actions.append(signal);

    if ((this->allowedActionTypes & activeOnState) != 0)
        actionType[signal->getName()] = action_types::activeOnState;
    else
        actionType[signal->getName()] = action_types::pulse;

    if (signal->getSize() > 1)
    {
        actionValue[signal->getName()] = LogicValue::getValue1(signal->getSize());
        actionParam1[signal->getName()] = -1;
        actionParam2[signal->getName()] = -1;
    }

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

        for (int i = 0 ; i < this->actions.count() ; i++)
        {
            if (this->actions[i].lock() == signal)
            {
                this->actions[i].reset();
            }
        }

        actionType  .remove(signal->getName());
        actionValue .remove(signal->getName());
        actionParam1.remove(signal->getName());
        actionParam2.remove(signal->getName());

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

        if (sig->getSize() == 1) // Single bit signal
        {
            switch (actionType[signame])
            {
            case action_types::set:
            case action_types::pulse:
            case action_types::activeOnState:
                sig->set();
                break;
            case action_types::reset:
                sig->resetValue();
                break;
            case action_types::assign:
                // Illegal, not handled
                break;
            }
        }
        else if ( (actionParam1[signame] != -1)  && (actionParam2[signame] == -1) ) // Single bit extract
        {
            LogicValue value = sig->getCurrentValue();

            switch (actionType[signame])
            {
            case action_types::set:
            case action_types::pulse:
            case action_types::activeOnState:
                value[actionParam1[signame]] = 1;
                break;
            case action_types::reset:
                value[actionParam1[signame]] = 0;
                break;
            case action_types::assign:
                // Illegal, not handled
                break;
            }

            sig->setCurrentValue(value); // Throws StatesException - current value is obtained from signal itself: size does match - ignored
        }
        else if ( (actionParam1[signame] == -1)  && (actionParam2[signame] == -1) ) // Whole vector affect
        {
            switch (actionType[signame])
            {
            case action_types::set:
                sig->set();
                break;
            case action_types::reset:
                sig->resetValue();
                break;
            case action_types::pulse:
            case action_types::activeOnState:
            case action_types::assign:
                try
                {
                    sig->setCurrentValue(actionValue[signame]); // Throws StatesException
                }
                catch (const StatesException& e)
                {
                    if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::size_mismatch) )
                    {
                        qDebug() << "(MachineActuatorComponent:) Error! Unable to set value of signal " << signame << ". Size does not match.";
                    }
                    else
                        throw;
                }
                break;
            }
        }
        else // Sub-range affect
        {
            LogicValue value = sig->getCurrentValue();
            int param1 = actionParam1[signame];
            int param2 = actionParam2[signame];

            switch (actionType[signame])
            {
            case action_types::set:
                for (int i = param2 ; i <= param1 ; i++)
                {
                    value[i] = 1;
                }
                break;
            case action_types::reset:
                for (int i = param2 ; i <= param1 ; i++)
                {
                    value[i] = 0;
                }
                break;
            case action_types::pulse:
            case action_types::activeOnState:
            case action_types::assign:
                for (int i = param2 ; i <= param1 ; i++)
                {
                    value[i] = actionValue[signame][i-param2];
                }
                break;
            }

            sig->setCurrentValue(value); // Throws StatesException - current value is obtained from signal itself: size does match - ignore
        }
    }
}

void MachineActuatorComponent::deactivateActions()
{
    foreach (shared_ptr<Signal> sig, this->getActions())
    {
        QString signame = sig->getName();

        if (sig->getSize() == 1) // Single bit signal
        {
            switch (actionType[signame])
            {
            case action_types::pulse:
            case action_types::activeOnState:
                sig->resetValue();
                break;
            case action_types::reset:
            case action_types::set:
                // Nothing to do, action is maintained
                break;
            case action_types::assign:
                // Illegal, not handled
                break;
            }
        }
        else if ( (actionParam1[signame] != -1)  && (actionParam2[signame] == -1) ) // Single bit extract
        {
            LogicValue value = sig->getCurrentValue();

            switch (actionType[signame])
            {
            case action_types::pulse:
            case action_types::activeOnState:
                value[actionParam1[signame]] = 0;
                break;
            case action_types::set:
            case action_types::reset:
                // Nothing to do, action is maintained
                break;
            case action_types::assign:
                // Illegal, not handled
                break;
            }

            sig->setCurrentValue(value); // Throws StatesException - current value is obtained from signal itself: size does match - ignore
        }
        else if ( (actionParam1[signame] == -1)  && (actionParam2[signame] == -1) ) // Whole vector affect
        {
            switch (actionType[signame])
            {
            case action_types::pulse:
            case action_types::activeOnState:
                sig->resetValue();
                break;
            case action_types::set:
            case action_types::reset:
            case action_types::assign:
                // Nothing to do, action is maintained
                break;
            }
        }
        else // Sub-range affect
        {
            LogicValue value = sig->getCurrentValue();

            int param1 = actionParam1[signame];
            int param2 = actionParam2[signame];

            switch (actionType[signame])
            {
            case action_types::pulse:
            case action_types::activeOnState:
                for (int i = param2 ; i <= param1 ; i++)
                {
                    value[i] = 0;
                }
                break;
            case action_types::set:
            case action_types::reset:
            case action_types::assign:
                // Nothing to do, action is maintained
                break;
            }

            sig->setCurrentValue(value); // Throws StatesException - current value is obtained from signal itself: size does match - ignore
        }
    }
}

void MachineActuatorComponent::setActionType(shared_ptr<Signal> signal, action_types type)
{
    this->cleanActionList();

    QString signame = signal->getName();

    if ( (signal->getSize() > 1) &&
         ( !( (actionParam1[signame] != -1) && (actionParam2[signame] == -1) ) )
       )
    {
        // We have to deal with affect value (single bit signals always have an implicit value)
        switch (type)
        {
        case action_types::reset:
        case action_types::set:
            // Switch to implicit
            actionValue.remove(signame);
            break;
        case action_types::activeOnState:
        case action_types::pulse:
        case action_types::assign:

            if ( (actionType[signame] == action_types::reset) ||
                 (actionType[signame] == action_types::set) )
            {
                if ((actionParam1[signame] == -1) && (actionParam2[signame] == -1)) // Whole range affect
                {
                    // Switch to explicit
                    actionValue[signame]  = LogicValue::getValue1(signal->getSize());
                }
                else if ((actionParam1[signame] != -1) && (actionParam2[signame] != -1)) // Sub-range affect
                {
                    // Switch to explicit
                    int affectSize = actionParam1[signame] - actionParam2[signame] + 1;
                    actionValue[signame]  = LogicValue::getValue1(affectSize);
                }
                // Else, single bit affect remains implicit
            }
            break;
        }
    }

    actionType[signame] = type;


    emit actionListChangedEvent();
}

bool MachineActuatorComponent::setActionValue(shared_ptr<Signal> signal, LogicValue value, int param1, int param2)
{
    if (signal->getSize() == 1)
    {
        // Size 1 signals always have implicit values
        return false;
    }
    else
    {
        QString signame = signal->getName();
        bool actionValueChanged = false;

        actionParam1[signame] = param1;
        actionParam2[signame] = param2;

        if ( (param1 != -1) && (param2 == -1))
        {
            // Single bit affect => implicit: value is ignored
            actionValue.remove(signame);

            if (actionType[signame] == action_types::assign) // Assign is illegal for single bit signals
                actionType[signame] = action_types::set;

            actionValueChanged = true;
        }
        else if (param1 != -1)
        {
            // Range affect

            LogicValue correctedValue = value; // Allow for shorter values (fill left with zeros)

            if ((int)value.getSize() < (param1-param2+1))
            {
                try
                {
                    correctedValue.resize(param1-param2+1); // Throws StatesException
                }
                catch (const StatesException& e)
                {
                    if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::resized_to_0) )
                    {
                        return false;
                    }
                    else
                        throw;
                }
            }

            if ( (int)correctedValue.getSize() == (param1-param2+1) )
            {
                actionValue[signal->getName()] = correctedValue;
                actionValueChanged = true;
            }
        }
        else
        {
            // Whole signal affect

            LogicValue correctedValue = value;

            if (value.getSize() < signal->getSize())  // Allow for shorter values (fill left with zeros)
            {
                try
                {
                    correctedValue.resize(signal->getSize()); // Throws StatesException
                }
                catch (const StatesException& e)
                {
                    if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::resized_to_0) )
                    {
                        return false;
                    }
                    else
                        throw;
                }
            }

            if (signal->getSize() == correctedValue.getSize())
            {
                actionValue[signal->getName()] = correctedValue;
                actionValueChanged = true;
            }
        }

        if (actionValueChanged)
        {
            emit actionListChangedEvent();
            return true;
        }
        else
            return false;
    }
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

int MachineActuatorComponent::getActionParam1(shared_ptr<Signal> variable)
{
    if (actionParam1.contains(variable->getName()))
        return actionParam1[variable->getName()];
    else
        return -1;
}

int MachineActuatorComponent::getActionParam2(shared_ptr<Signal> variable)
{
    if (actionParam2.contains(variable->getName()))
        return actionParam2[variable->getName()];
    else
        return -1;
}


