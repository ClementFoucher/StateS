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
#include "machine.h"

// Qt classes
#include <QGraphicsItem>

// StateS classes
#include "input.h"
#include "output.h"


Machine::Machine()
{
}

QList<shared_ptr<Input> > Machine::getInputs() const
{
    return this->inputs.values();
}

QList<shared_ptr<Output> > Machine::getOutputs() const
{
    return this->outputs.values();
}

QList<shared_ptr<Signal> > Machine::getLocalVariables() const
{
    return this->localVariables.values();
}

QList<shared_ptr<Signal> > Machine::getConstants() const
{
    return this->constants.values();
}

QList<shared_ptr<Signal> > Machine::getWrittableSignals() const
{
    QList<shared_ptr<Signal>> writtableVariables;

    writtableVariables += this->getOutputsAsSignals();
    writtableVariables += this->localVariables.values();

    return writtableVariables;
}

QList<shared_ptr<Signal> > Machine::getReadableSignals() const
{
    QList<shared_ptr<Signal>> readableSignals;

    readableSignals += this->getInputsAsSignals();
    readableSignals += this->localVariables.values();
    readableSignals += this->constants.values();

    return readableSignals;
}

QList<shared_ptr<Signal> > Machine::getReadableVariableSignals() const
{
    QList<shared_ptr<Signal>> readableVariables;

    readableVariables += this->getInputsAsSignals();
    readableVariables += this->localVariables.values();

    return readableVariables;
}

QList<shared_ptr<Signal> > Machine::getVariablesSignals() const
{
    QList<shared_ptr<Signal>> allVariables;

    allVariables += this->getInputsAsSignals();
    allVariables += this->getOutputsAsSignals();
    allVariables += this->localVariables.values();

    return allVariables;
}

QList<shared_ptr<Signal> > Machine::getIoSignals() const
{
    QList<shared_ptr<Signal>> IOs;

    IOs += this->getInputsAsSignals();
    IOs += this->getOutputsAsSignals();

    return IOs;
}

QList<shared_ptr<Signal> > Machine::getAllSignals() const
{
    QList<shared_ptr<Signal>> allSignals;

    allSignals += this->getInputsAsSignals();
    allSignals += this->getOutputsAsSignals();
    allSignals += localVariables.values();
    allSignals += constants.values();

    return allSignals;
}

void Machine::clear()
{
    this->inputs.clear();
    this->outputs.clear();
    this->localVariables.clear();
    this->constants.clear();
}

bool Machine::isEmpty() const
{
    if (this->inputs.isEmpty() &&
        this->outputs.isEmpty() &&
        this->localVariables.isEmpty() &&
        this->constants.isEmpty()
       )
        return true;
    else
        return false;
}

QGraphicsItem* Machine::getComponentVisualization()
{
    QGraphicsItem* visu = new QGraphicsItemGroup();

    uint componentWidth = 0;
    uint componentHeigh = 0;
    uint horizontalSignalsSpace = 50;

    uint inputsXSize = 0;
    QList<shared_ptr<Input>> inputs = this->getInputs();
    for(int i = 0 ; i < inputs.count() ; i++)
    {
        QGraphicsTextItem* text = new QGraphicsTextItem(inputs[i]->getText(), visu);
        text->setPos(0, 25 + i*20);
        if (text->boundingRect().width() > inputsXSize)
            inputsXSize = text->boundingRect().width();

        uint lineHeigh = text->pos().y() + text->boundingRect().height()/2;
        new QGraphicsLineItem(-10, lineHeigh, 0, lineHeigh, visu);
    }
    componentWidth += inputsXSize;

    uint outputsXSize = 0;
    QList<shared_ptr<Output>> outputs = this->getOutputs();
    QList<QGraphicsTextItem*> graphicsOutputs;
    for(int i = 0 ; i < outputs.count() ; i++)
    {
        QGraphicsTextItem* text = new QGraphicsTextItem(outputs[i]->getText(), visu);
        text->setPos(inputsXSize + 50, 25 + i*20);
        if (text->boundingRect().width() > outputsXSize)
            outputsXSize = text->boundingRect().width();

        graphicsOutputs.append(text);
    }
    componentWidth += horizontalSignalsSpace + outputsXSize;

    foreach(QGraphicsTextItem* text, graphicsOutputs)
    {
        text->setX(componentWidth - text->boundingRect().width());

        uint lineHeigh = text->pos().y() + text->boundingRect().height()/2;
        new QGraphicsLineItem(componentWidth, lineHeigh, componentWidth + 10, lineHeigh, visu);
    }

    uint maxSignalsCount = max(this->inputs.count(), this->outputs.count());

    QGraphicsTextItem* title = new QGraphicsTextItem(visu);
    title->setHtml("<b>" + tr("Machine") + "</b>");

    if (componentWidth <= title->boundingRect().width() + horizontalSignalsSpace)
    {
        componentWidth = title->boundingRect().width() + horizontalSignalsSpace;
    }

    title->setPos((componentWidth-title->boundingRect().width())/2, 5);

    componentHeigh = 40 + 20*maxSignalsCount;

    QPolygonF borderPolygon;
    borderPolygon.append(QPoint(0,              0));
    borderPolygon.append(QPoint(componentWidth, 0));
    borderPolygon.append(QPoint(componentWidth, componentHeigh));
    borderPolygon.append(QPoint(0,              componentHeigh));

    new QGraphicsPolygonItem(borderPolygon, visu);

    return visu;
}

shared_ptr<Signal> Machine::addSignal(signal_type type, const QString& name)
{
    // First check if name doesn't already exist
    foreach (shared_ptr<Signal> signal, getAllSignals())
    {
        if (signal->getName() == name)
            return nullptr;
    }

    // Determine list to reference signal in
    shared_ptr<Signal> result;
    switch(type)
    {
    case signal_type::Input:
        inputs[name] = shared_ptr<Input>(new Input(name));
        result = inputs[name];
        emit inputListChangedEvent();
        break;
    case signal_type::Output:
        outputs[name] = shared_ptr<Output>(new Output(name));
        result = outputs[name];
        emit outputListChangedEvent();
        break;
    case signal_type::LocalVariable:
        result = shared_ptr<Signal>(new Signal(name));
        localVariables[name] = result;
        emit localVariableListChangedEvent();
        break;
    case signal_type::Constant:
        result = shared_ptr<Signal>(new Signal(name, false, true));
        constants[name] = result;
        emit constantListChangedEvent();
        break;
    }

    return result;
}

bool Machine::deleteSignal(const QString& name)
{
    if (inputs.contains(name))
    {
        inputs.remove(name);
        emit inputListChangedEvent();

        return true;
    }
    else if (outputs.contains(name))
    {
        outputs.remove(name);
        emit outputListChangedEvent();

        return true;
    }
    else if (localVariables.contains(name))
    {
        localVariables.remove(name);
        emit localVariableListChangedEvent();

        return true;
    }
    else if (constants.contains(name))
    {
        constants.remove(name);
        emit constantListChangedEvent();

        return true;
    }
    else
        return false;
}

bool Machine::renameSignal(const QString& oldName, const QString& newName)
{
    QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

    if ( !allSignals.contains(oldName) ) // First check if signal exists
        return false;
    else if (oldName == newName) // Rename to same name is always success
        return true;
    else if ( allSignals.contains(newName) ) // Do not allow rename to existing name
        return false;
    else
    {
        // Renaming process
        shared_ptr<Signal> itemToRename = allSignals[oldName];
        itemToRename->setName(newName);

        // Update map
        if (inputs.contains(oldName))
        {
            inputs.remove(oldName);
            inputs[newName] = dynamic_pointer_cast<Input>(itemToRename);
            emit inputListChangedEvent();
        }
        else if (outputs.contains(oldName))
        {
            outputs.remove(oldName);
            outputs[newName] = dynamic_pointer_cast<Output>(itemToRename);
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
    QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

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
    QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

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

QList<shared_ptr<Signal> > Machine::getInputsAsSignals() const
{
    QList<shared_ptr<Signal>> signalInputs;

    foreach(shared_ptr<Input> input, this->inputs)
    {
        signalInputs.append(dynamic_pointer_cast<Signal>(input));
    }

    return signalInputs;
}

QList<shared_ptr<Signal> > Machine::getOutputsAsSignals() const
{
    QList<shared_ptr<Signal>> signalOutputs;

    foreach(shared_ptr<Output> output, this->outputs)
    {
        signalOutputs.append(dynamic_pointer_cast<Signal>(output));
    }

    return signalOutputs;
}

QHash<QString, shared_ptr<Signal> > Machine::getAllSignalsMap() const
{
    QHash<QString, shared_ptr<Signal>> allSignals;

    foreach (shared_ptr<Signal> signal, getAllSignals())
    {
        allSignals[signal->getName()] = signal;
    }

    return allSignals;
}
