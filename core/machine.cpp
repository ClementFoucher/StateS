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
#include "logicvariable.h"

Machine::Machine()
{

}

Machine::~Machine()
{
    qDeleteAll(inputs);
    qDeleteAll(outputs);
    qDeleteAll(localVariables);
}


QList<Input*> Machine::getInputs() const
{
    return inputs.values();
}


QList<Output*> Machine::getOutputs() const
{
    return outputs.values();
}

QList<LogicVariable *> Machine::getLocalVariables() const
{
    return localVariables.values();
}

QList<LogicVariable *> Machine::getWrittableVariables() const
{
    QList<LogicVariable*> variables;

    QList<Output*> outputList = outputs.values();
    variables += *reinterpret_cast<QList<LogicVariable*>*> (&outputList);
    variables += localVariables.values();

    return variables;
}

QList<LogicVariable*> Machine::getReadableVariables() const
{
    QList<LogicVariable*> variables;

    QList<Input*> inputList = inputs.values();

    variables += *reinterpret_cast<QList<LogicVariable*>*> (&inputList);
    variables += localVariables.values();

    return variables;
}

QList<LogicVariable*> Machine::getAllVariables() const
{
    QList<LogicVariable*> variables;

    QList<Input*> inputList = inputs.values();
    QList<Output*> outputList = outputs.values();

    variables += *reinterpret_cast<QList<LogicVariable*>*> (&inputList);
    variables += *reinterpret_cast<QList<LogicVariable*>*> (&outputList);
    variables += localVariables.values();

    return variables;

}

bool Machine::addInput(const QString& name)
{
    Input* newInput = new Input(name);

    if (addVariable(reinterpret_cast<QHash<QString, LogicVariable*>*> (&inputs), newInput))
    {
        emit InputListChangedEvent();
        return true;
    }
    else
    {
        delete newInput;
        return false;
    }
}

bool Machine::addOutput(const QString& name)
{
    Output* newOutput = new Output(name);

    if (addVariable(reinterpret_cast<QHash<QString, LogicVariable*>*> (&outputs), newOutput))
    {
        emit OutputListChangedEvent();
        return true;
    }
    else
    {
        delete newOutput;
        return false;
    }
}

bool Machine::addLocalVariable(const QString& name)
{
    LogicVariable* newVariable= new LogicVariable(name);

    if (addVariable(&localVariables, newVariable))
    {
        emit LocalVariableListChangedEvent();
        return true;
    }
    else
    {
        delete newVariable;
        return false;
    }
}

bool Machine::deleteInput(const QString& name)
{
    if (inputs.contains(name))
    {
        IO * toDelete = inputs[name];

        inputs.remove(name);

        delete toDelete;

        emit InputListChangedEvent();

        return true;
    }
    else
        return false;
}

bool Machine::deleteOutput(const QString& name)
{
    if (outputs.contains(name))
    {
        IO * toDelete = outputs[name];

        outputs.remove(name);

        delete toDelete;

        emit OutputListChangedEvent();

        return true;
    }
    else
        return false;
}

bool Machine::deleteLocalVariable(const QString& name)
{
    if (localVariables.contains(name))
    {
        LogicVariable* toDelete = localVariables[name];

        localVariables.remove(name);

        delete toDelete;

        emit LocalVariableListChangedEvent();

        return true;
    }
    else
        return false;
}

bool Machine::renameInput(const QString& oldName, const QString& newName)
{
    bool success = renameVariable(reinterpret_cast<QHash< QString, LogicVariable*>*> (&inputs), oldName, newName);

    if (success)
        emit InputListChangedEvent();

    return success;
}

bool Machine::renameOutput(const QString& oldName, const QString& newName)
{
    bool success = renameVariable(reinterpret_cast<QHash< QString, LogicVariable*>*> (&outputs), oldName, newName);

    if (success)
        emit OutputListChangedEvent();

    return success;
}

bool Machine::renameLocalVariable(const QString& oldName, const QString& newName)
{
    bool success = renameVariable(&localVariables, oldName, newName);

    if (success)
        emit LocalVariableListChangedEvent();

    return success;
}

bool Machine::renameVariable(QHash< QString, LogicVariable*>* list, const QString& oldName, const QString& newName)
{
    LogicVariable* itemToRename = (*list)[oldName];

    if (itemToRename == nullptr)
        return false;
    else if (oldName == newName)
        return true;
    else
    {
        foreach (LogicVariable* var, getAllVariables())
        {
            if (var->getName() == newName)
                return false;
        }

        itemToRename->setName(newName);
        if (list->remove(oldName) == 0)
            return false;
        else
        {
            (*list)[newName] = itemToRename;
            return true;

        }
    }
}

bool Machine::addVariable(QHash< QString, LogicVariable*>* list, LogicVariable* newVariable)
{
    foreach (LogicVariable* var, getAllVariables())
    {
        if (var->getName() == newVariable->getName())
            return false;
    }


    (*list)[newVariable->getName()] = newVariable;

    return true;

}
