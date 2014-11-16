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

#include "machineactuatorelement.h"

#include "logicvariable.h"
#include "machine.h"

MachineActuatorElement::MachineActuatorElement(Machine* owningMachine) :
    MachineElement(owningMachine)
{
}


void MachineActuatorElement::removeAction(LogicVariable* variable)
{
    disconnect(variable, SIGNAL(renamedEvent()), this, SIGNAL(elementConfigurationChanged()));
    disconnect(variable, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(removeAction(LogicVariable*)));
    disconnect(variable, SIGNAL(stateChangedEvent()), this, SIGNAL(elementConfigurationChanged()));

    actions.removeAll(variable);
    actionType.remove(variable);
    emit elementConfigurationChanged();
}

QList<LogicVariable *> MachineActuatorElement::getActions() const
{
    return actions;
}

void MachineActuatorElement::setActions(const QList<LogicVariable*>* newActions)
{
    actions = *newActions;

    foreach (LogicVariable* var, actions)
    {
        actionType[var] = LogicVariable::action_types::pulse;
    }
}


void MachineActuatorElement::clearActions()
{
    QList<LogicVariable*> actionsToDelete = actions;

    foreach(LogicVariable* var, actionsToDelete)
    {
        removeAction(var);
    }

    actionType.clear();
}

void MachineActuatorElement::addAction(LogicVariable* variable)
{
    actions.append(variable);
    actionType[variable] = LogicVariable::action_types::pulse;

    connect(variable, SIGNAL(renamedEvent()), this, SIGNAL(elementConfigurationChanged()));
    connect(variable, SIGNAL(deletedEvent(LogicVariable*)), this, SLOT(removeAction(LogicVariable*)));
    connect(variable, SIGNAL(stateChangedEvent()), this, SIGNAL(elementConfigurationChanged()));

    emit elementConfigurationChanged();
}

void MachineActuatorElement::addAction(const QString& variableName)
{
    foreach (LogicVariable* var, getOwningMachine()->getWrittableVariables())
    {
        if (var->getName() == variableName)
        {
            addAction(var);
            break;
        }
    }
}

void MachineActuatorElement::removeAction(const QString& variableName)
{
    LogicVariable* foundVar = nullptr;

    foreach (LogicVariable* var, actions)
    {
        if (var->getName() == variableName)
        {
            foundVar = var;
            break;
        }
    }

    if (foundVar != nullptr)
        removeAction(foundVar);
}

void MachineActuatorElement::setActionType(LogicVariable* variable, LogicVariable::action_types type)
{
    actionType[variable] = type;

    emit elementConfigurationChanged();
}

LogicVariable::action_types MachineActuatorElement::getActionType(LogicVariable* variable)
{
    return actionType[variable];
}
