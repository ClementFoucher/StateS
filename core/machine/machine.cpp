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

    this->rebuildComponentVisualization();
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

/**
 * @brief Machine::getComponentVisualization
 * Object calling this function takes ownership op
 * visu. We need to rebuild it, but silently as this
 * is not an update, just a kind of copy.
 * @return
 */
QGraphicsItem* Machine::getComponentVisualization()
{
    QGraphicsItem* currentVisu = this->componentVisu;
    this->componentVisu = nullptr;

    inhibateEvent = true;
    rebuildComponentVisualization();
    inhibateEvent = false;

    return currentVisu;
}

void Machine::rebuildComponentVisualization()
{
    // /!\ QGraphicsItemGroup bounding box seems not to be updated
    // if item is added using its constructor's parent parameter

    delete this->componentVisu;
    this->componentVisu = nullptr;

    QGraphicsItemGroup* visu = new QGraphicsItemGroup();

    //
    // Main sizes

    qreal signalsLinesWidth = 20;
    qreal horizontalSignalsNamesSpacer = 50;
    qreal verticalElementsSpacer = 5;
    qreal busesLineHeight = 10;
    qreal busesLineWidth = 5;


    //
    // Draw inputs

    QGraphicsItemGroup* inputsGroup = new QGraphicsItemGroup();

    {
        // Items position wrt. subgroup:
        // All items @ Y = 0, and rising
        // Signals names @ X > 0
        // Lines @ X < 0

        QList<shared_ptr<Input>> inputs = this->getInputs();

        qreal currentInputY = 0;
        for(int i = 0 ; i < inputs.count() ; i++)
        {
            QGraphicsTextItem* text = new QGraphicsTextItem(inputs[i]->getText());//, inputsGroup);
            inputsGroup->addToGroup(text);
            text->setPos(0, currentInputY);

            qreal currentLineY = currentInputY + text->boundingRect().height()/2;
            inputsGroup->addToGroup(new QGraphicsLineItem(-signalsLinesWidth, currentLineY, 0, currentLineY));//, inputsGroup);

            if (inputs[i]->getSize() > 1)
            {
                inputsGroup->addToGroup(new QGraphicsLineItem(-signalsLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, -signalsLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
                QGraphicsTextItem* sizeText = new QGraphicsTextItem(QString::number(inputs[i]->getSize()));
                inputsGroup->addToGroup(sizeText);
                sizeText->setPos(-signalsLinesWidth/2 - sizeText->boundingRect().width(), currentLineY - sizeText->boundingRect().height());
            }

            currentInputY += text->boundingRect().height();
        }
    }


    //
    // Draw outputs

    QGraphicsItemGroup* outputsGroup = new QGraphicsItemGroup();

    {
        // Items position wrt. subgroup:
        // All items @ Y = 0, and rising
        // Signals names @ X < 0
        // Lines @ X > 0

        QList<shared_ptr<Output>> outputs = this->getOutputs();

        qreal currentOutputY = 0;
        for(int i = 0 ; i < outputs.count() ; i++)
        {
            QGraphicsTextItem* text = new QGraphicsTextItem(outputs[i]->getText()); //, outputsGroup);
            outputsGroup->addToGroup(text);
            text->setPos(-text->boundingRect().width(), currentOutputY);

            qreal currentLineY = currentOutputY + text->boundingRect().height()/2;
            outputsGroup->addToGroup(new QGraphicsLineItem(0, currentLineY, signalsLinesWidth, currentLineY));

            if (outputs[i]->getSize() > 1)
            {
                outputsGroup->addToGroup(new QGraphicsLineItem(signalsLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, signalsLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
                QGraphicsTextItem* sizeText = new QGraphicsTextItem(QString::number(outputs[i]->getSize()));
                outputsGroup->addToGroup(sizeText);
                sizeText->setPos(signalsLinesWidth/2, currentLineY - sizeText->boundingRect().height());
            }

            currentOutputY += text->boundingRect().height();
        }
    }

    //
    // Draw component name

    QGraphicsTextItem* title = new QGraphicsTextItem();

    {
        title->setHtml("<b>" + tr("Machine") + "</b>");
    }


    //
    // Compute component size

    qreal componentWidth;
    qreal componentHeight;

    {
        // Width

        qreal inputsNamesWidth = inputsGroup->boundingRect().width() - signalsLinesWidth;
        qreal outputsNamesWidth = outputsGroup->boundingRect().width() - signalsLinesWidth;

        componentWidth = inputsNamesWidth + horizontalSignalsNamesSpacer + outputsNamesWidth;

        if (componentWidth <= title->boundingRect().width() + horizontalSignalsNamesSpacer)
        {
            componentWidth = title->boundingRect().width() + horizontalSignalsNamesSpacer;
        }

        // Height

        qreal maxSignalsHeight = max(inputsGroup->boundingRect().height(), outputsGroup->boundingRect().height());

        componentHeight =
                verticalElementsSpacer +
                title->boundingRect().height() +
                verticalElementsSpacer +
                maxSignalsHeight +
                verticalElementsSpacer;

    }

    //
    // Draw component border

    QGraphicsPolygonItem* border = nullptr;

    {
        QPolygonF borderPolygon;
        borderPolygon.append(QPoint(0,              0));
        borderPolygon.append(QPoint(componentWidth, 0));
        borderPolygon.append(QPoint(componentWidth, componentHeight));
        borderPolygon.append(QPoint(0,              componentHeight));

        border = new QGraphicsPolygonItem(borderPolygon);
    }

    //
    // Place components in main group

    {
        // Items position wrt. main group:
        // Component top left corner @ (0; 0)

        visu->addToGroup(border);
        visu->addToGroup(title);
        visu->addToGroup(inputsGroup);
        visu->addToGroup(outputsGroup);


        border->setPos(0, 0);

        title->setPos( (componentWidth-title->boundingRect().width())/2, verticalElementsSpacer);

        qreal verticalSignalsNameOffset = title->boundingRect().bottom() + verticalElementsSpacer;

        qreal inoutsDeltaHeight = inputsGroup->boundingRect().height() - outputsGroup->boundingRect().height();
        qreal additionalInputsOffet  = (inoutsDeltaHeight > 0 ? 0 : -inoutsDeltaHeight/2);
        qreal additionalOutputsOffet = (inoutsDeltaHeight < 0 ? 0 : inoutsDeltaHeight/2);

        inputsGroup-> setPos(0,              verticalSignalsNameOffset + additionalInputsOffet);
        outputsGroup->setPos(componentWidth, verticalSignalsNameOffset + additionalOutputsOffet);
    }


    //
    // Done

    this->componentVisu = visu;

    if (!inhibateEvent)
        emit componentVisualizationUpdatedEvent();
}

shared_ptr<Signal> Machine::addSignal(signal_type type, const QString& name)
{
    // First check if name doesn't already exist
    foreach (shared_ptr<Signal> signal, getAllSignals())
    {
        if (signal->getName() == name)
            return nullptr;
    }

    // Then check for illegal characters
    QString cleanName = name;
    if (!this->cleanSignalName(cleanName))
        return nullptr;

    // Determine list to reference signal in
    shared_ptr<Signal> result;
    switch(type)
    {
    case signal_type::Input:
        inputs[name] = shared_ptr<Input>(new Input(name));
        result = inputs[name];
        this->rebuildComponentVisualization();
        emit inputListChangedEvent();
        break;
    case signal_type::Output:
        outputs[name] = shared_ptr<Output>(new Output(name));
        result = outputs[name];
        this->rebuildComponentVisualization();
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
    bool result;

    if (inputs.contains(name))
    {
        inputs.remove(name);
        this->rebuildComponentVisualization();
        emit inputListChangedEvent();

        result = true;
    }
    else if (outputs.contains(name))
    {
        outputs.remove(name);
        this->rebuildComponentVisualization();
        emit outputListChangedEvent();

        result = true;
    }
    else if (localVariables.contains(name))
    {
        localVariables.remove(name);
        emit localVariableListChangedEvent();

        result = true;
    }
    else if (constants.contains(name))
    {
        constants.remove(name);
        emit constantListChangedEvent();

        result = true;
    }
    else
        result = false;

    return result;
}

bool Machine::renameSignal(const QString& oldName, const QString& newName)
{
    QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

    QString nonConstNewName = newName;

    if ( !allSignals.contains(oldName) ) // First check if signal exists
        return false;
    else if (oldName == newName) // Rename to same name is always success
        return true;
    else if ( allSignals.contains(newName) ) // Do not allow rename to existing name
        return false;
    else if ( ! this->cleanSignalName(nonConstNewName) ) // Check for name correctness
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
            this->rebuildComponentVisualization();

            emit inputListChangedEvent();
        }
        else if (outputs.contains(oldName))
        {
            outputs.remove(oldName);
            outputs[newName] = dynamic_pointer_cast<Output>(itemToRename);
            this->rebuildComponentVisualization();

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
                this->rebuildComponentVisualization();
                emit inputListChangedEvent();
            }
            else if (outputs.contains(name))
            {
                this->rebuildComponentVisualization();
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

/**
 * @brief Machine::cleanSignalName
 * @param nameToClean
 * @return
 *  True if name was clean,
 *  False if string has been cleaned.
 */
bool Machine::cleanSignalName(QString& nameToClean) const
{
    bool clean = true;
    QString cleanName;

    foreach (QChar c, nameToClean)
    {
        if ( ( (c.isLetterOrNumber()) ) ||
             ( (c == '_')             ) ||
             ( (c == '#')             ) ||
             ( (c == '@')             ) ||
             ( (c == '-')             ) ||
             ( (c.isSpace() )         )
           )
        {
            cleanName += c;
        }
        else
        {
            clean = false;
        }
    }

    if (!clean)
        nameToClean = cleanName;

    return clean;
}

QString Machine::getUniqueSignalName(const QString& prefix) const
{
    QString baseName = prefix;
    this->cleanSignalName(baseName);

    QString currentName;

    uint i = 0;
    bool nameIsValid = false;

    while (!nameIsValid)
    {
        currentName = baseName + QString::number(i);

        nameIsValid = true;
        foreach(shared_ptr<Signal> colleage, this->getAllSignals())
        {
            if (colleage->getName() == currentName)
            {
                nameIsValid = false;
                i++;
                break;
            }
        }
    }

    return currentName;
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
