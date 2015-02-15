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

#include "machine.h"

#include "input.h"
#include "output.h"
#include "io.h"
#include "signal.h"

Machine::Machine()
{

}

Machine::~Machine()
{
    qDeleteAll(inputs);
    qDeleteAll(outputs);
    qDeleteAll(localVariables);
    qDeleteAll(constants);
}

QList<Input*> Machine::getInputs() const
{
    return inputs.values();
}

QList<Output*> Machine::getOutputs() const
{
    return outputs.values();
}

QList<Signal*> Machine::getLocalVariables() const
{
    return localVariables.values();
}

QList<Signal*> Machine::getConstants() const
{
    return constants.values();
}

QList<Signal *> Machine::getWrittableSignals() const
{
    QList<Signal*> writtableVariables;

    QList<Output*> outputList = outputs.values();
    writtableVariables += *reinterpret_cast<QList<Signal*>*> (&outputList);
    writtableVariables += localVariables.values();

    return writtableVariables;
}

QList<Signal*> Machine::getReadableSignals() const
{
    QList<Signal*> readableVariables;

    QList<Input*> inputList = inputs.values();

    readableVariables += *reinterpret_cast<QList<Signal*>*> (&inputList);
    readableVariables += localVariables.values();
    readableVariables += constants.values();

    return readableVariables;
}

QList<Signal*> Machine::getAllVariables() const
{
    QList<Signal*> allVariables;

    QList<Input*> inputList = inputs.values();
    QList<Output*> outputList = outputs.values();

    allVariables += *reinterpret_cast<QList<Signal*>*> (&inputList);
    allVariables += *reinterpret_cast<QList<Signal*>*> (&outputList);
    allVariables += localVariables.values();

    return allVariables;
}

QList<Signal*> Machine::getAllSignals() const
{
    QList<Signal*> allSignals;

    QList<Input*> inputList = inputs.values();
    QList<Output*> outputList = outputs.values();

    allSignals += *reinterpret_cast<QList<Signal*>*> (&inputList);
    allSignals += *reinterpret_cast<QList<Signal*>*> (&outputList);
    allSignals += localVariables.values();
    allSignals += constants.values();

    return allSignals;
}

bool Machine::addSignal(signal_types type, const QString& name)
{
    // First check if name doesn't already exist
    foreach (Signal* signal, getAllSignals())
    {
        if (signal->getName() == name)
            return false;
    }

    // Determine list to reference signal in
    switch(type)
    {
    case signal_types::Input:
        inputs[name] = new Input(name);
        emit inputListChangedEvent();
        break;
    case signal_types::Output:
        outputs[name] = new Output(name);
        emit outputListChangedEvent();
        break;
    case signal_types::LocalVariable:
        localVariables[name] = new Signal(name);
        emit localVariableListChangedEvent();
        break;
    case signal_types::Constant:
        constants[name] = new Signal(name, false, true);
        emit constantListChangedEvent();
        break;
    }

    return true;
}

bool Machine::deleteSignal(const QString& name)
{
    if (inputs.contains(name))
    {
        IO * toDelete = inputs[name];
        inputs.remove(name);
        delete toDelete;
        emit inputListChangedEvent();
        return true;
    }
    else if (outputs.contains(name))
    {
        IO * toDelete = outputs[name];
        outputs.remove(name);
        delete toDelete;
        emit outputListChangedEvent();
        return true;
    }
    else if (localVariables.contains(name))
    {
        Signal* toDelete = localVariables[name];
        localVariables.remove(name);
        delete toDelete;
        emit localVariableListChangedEvent();
        return true;
    }
    else if (constants.contains(name))
    {
        Signal* toDelete = constants[name];
        constants.remove(name);
        delete toDelete;
        emit constantListChangedEvent();
        return true;
    }
    else
        return false;
}

bool Machine::renameSignal(const QString& oldName, const QString& newName)
{
    QHash<QString, Signal*> allSignals = getAllSignalsMap();

    if ( !allSignals.contains(oldName) ) // First check if signal exists
        return false;
    else if (oldName == newName) // Rename to same name is always success
        return true;
    else if ( allSignals.contains(newName) ) // Do not allow rename to existing name
        return false;
    else
    {
        // Renaming process
        Signal* itemToRename = allSignals[oldName];
        itemToRename->setName(newName);

        // Update map
        if (inputs.contains(oldName))
        {
            inputs.remove(oldName);
            inputs[newName] = (Input*)itemToRename;
            emit inputListChangedEvent();
        }
        else if (outputs.contains(oldName))
        {
            outputs.remove(oldName);
            outputs[newName] = (Output*)itemToRename;
            emit outputListChangedEvent();
        }
        else if (localVariables.contains(oldName))
        {
            localVariables.remove(oldName);
            localVariables[newName] = itemToRename;
            emit localVariableListChangedEvent();
        }
        else if (constants.contains(oldName))
        {
            constants.remove(oldName);
            constants[newName] = itemToRename;
            emit constantListChangedEvent();
        }
        else // Should not happen as we checked all lists
            return false;

        return true;
    }
}

bool Machine::resizeSignal(const QString &name, uint newSize)
{
    QHash<QString, Signal*> allSignals = getAllSignalsMap();

    if ( !allSignals.contains(name) ) // First check if signal exists
        return false;
    else
    {
        if (allSignals[name]->resize(newSize) == true)
        {
            if (inputs.contains(name))
            {
                emit inputListChangedEvent();
            }
            else if (outputs.contains(name))
            {
                emit outputListChangedEvent();
            }
            else if (localVariables.contains(name))
            {
                emit localVariableListChangedEvent();
            }
            else if (constants.contains(name))
            {
                emit constantListChangedEvent();
            }
            else // Should not happen as we checked all lists
                return false;

            return true;
        }
        else
            return false;
    }
}

bool Machine::changeSignalInitialValue(const QString &name, LogicValue newValue)
{
    QHash<QString, Signal*> allSignals = getAllSignalsMap();

    if ( !allSignals.contains(name) ) // First check if signal exists
        return false;
    else
    {
        if (allSignals[name]->setInitialValue(newValue) == true)
        {
            if (inputs.contains(name))
            {
                emit inputListChangedEvent();
            }
            else if (outputs.contains(name))
            {
                emit outputListChangedEvent();
            }
            else if (localVariables.contains(name))
            {
                emit localVariableListChangedEvent();
            }
            else if (constants.contains(name))
            {
                emit constantListChangedEvent();
            }
            else // Should not happen as we checked all lists
                return false;

            return true;
        }
        else
            return false;
    }
}

QHash<QString, Signal*> Machine::getAllSignalsMap() const
{
    QHash<QString, Signal*> allSignals;

    foreach (Signal* signal, getAllSignals())
    {
        allSignals[signal->getName()] = signal;
    }

    return allSignals;
}
