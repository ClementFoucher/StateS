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

#include <QDebug>
#include <QFile>
#include <QXmlStreamWriter>
#include <QDomDocument>

#include "fsm.h"

#include <logicequation.h>
#include "fsmstate.h"
#include "fsmtransition.h"
#include "logicvariable.h"
#include "io.h"
#include "input.h"
#include "output.h"
#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"

Fsm::Fsm()
{

}

Fsm::Fsm(const QString& filePath)
{
    parseXML(filePath);
}

Fsm::~Fsm()
{
    // Get lists, then delete them (delete process calls back events that changes the lists, have to copy)
    QList<FsmTransition*> transitionsToDelete = transitions;
    qDeleteAll(transitionsToDelete);

    QList<FsmState*> statesToDelete = states;
    qDeleteAll(statesToDelete);
}

Machine::type Fsm::getType() const
{
    return Machine::type::FSM;
}

const QList<FsmState *>& Fsm::getStates() const
{
    return states;
}

void Fsm::addState(FsmState* state)
{
    states.append(state);
}

void Fsm::removeState(FsmState* state)
{
    states.removeAll(state);

    if (state == initialState)
        setInitialState(nullptr);
}

void Fsm::addTransition(FsmTransition* transition)
{
    transitions.append(transition);
}

void Fsm::removeTransition(FsmTransition* transition)
{
    transitions.removeAll(transition);
}

void Fsm::simulationModeChanged()
{
    foreach (LogicVariable* var, getReadableVariables())
    {
        emit var->stateChangedEvent();
    }
}

const QList<FsmTransition *>& Fsm::getTransitions() const
{
    return transitions;
}

void Fsm::setInitialState(FsmState* newInitialState)
{
    if (newInitialState != initialState)
    {
        FsmState* previousInitialState = initialState;

        initialState = newInitialState;

        if (initialState != nullptr)
            emit initialState->elementConfigurationChanged();

        if (previousInitialState != nullptr)
            emit previousInitialState->elementConfigurationChanged();
    }
}

FsmState* Fsm::getInitialState() const
{
    return initialState;
}

void Fsm::saveMachine(const QString& path)
{
    QFile* file = new QFile(path);
    file->open(QIODevice::WriteOnly);

    QXmlStreamWriter stream(file);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("FSM");


    stream.writeStartElement("Signals");
    foreach (LogicVariable* var, getAllVariables())
    {
        // Type
        if (dynamic_cast<Input*>(var) != nullptr)
            stream.writeStartElement("Input");
        else if (dynamic_cast<Output*>(var) != nullptr)
            stream.writeStartElement("Output");
        else
            stream.writeStartElement("Variable");

        // Name
        stream.writeAttribute("Name", var->getName());

        stream.writeEndElement();
    }
    stream.writeEndElement(); // Signals


    stream.writeStartElement("States");
    foreach (FsmState* state, states)
    {
        stream.writeStartElement("State");

        // Name
        stream.writeAttribute("Name", state->getName());

        // Initial
        if (state->isInitial())
            stream.writeAttribute("IsInitial", "true");

        // Position
        stream.writeAttribute("X", QString::number(state->getGraphicalRepresentation()->scenePos().x()));
        stream.writeAttribute("Y", QString::number(state->getGraphicalRepresentation()->scenePos().y()));

        // Actions
        foreach (LogicVariable* action, state->getActions())
        {
            stream.writeStartElement("Action");

            if ((dynamic_cast<Output*> (action)) != nullptr)
                stream.writeAttribute("Type", "Output");
            else
                stream.writeAttribute("Type", "Variable");

            stream.writeAttribute("Name", action->getName());

            stream.writeEndElement(); // Action
        }

        stream.writeEndElement(); // State
    }
    stream.writeEndElement(); // Transitions

    stream.writeStartElement("Transitions");
    foreach (FsmTransition* transition, transitions)
    {
        stream.writeStartElement("Transition");

        stream.writeAttribute("Source", transition->getSource()->getName());
        stream.writeAttribute("Target", transition->getTarget()->getName());

        // Deal with equations
        stream.writeStartElement("Condition");
        writeLogicEquation(stream, transition->getCondition());
        stream.writeEndElement(); // Condition

        // Actions
        stream.writeStartElement("Actions");
        foreach (LogicVariable* action, transition->getActions())
        {
            stream.writeStartElement("Action");

            if ((dynamic_cast<Output*> (action)) != nullptr)
                stream.writeAttribute("Type", "Output");
            else
                stream.writeAttribute("Type", "Variable");

            stream.writeAttribute("Name", action->getName());

            stream.writeEndElement(); // Action
        }
        stream.writeEndElement(); // Actions

        stream.writeEndElement(); // Transition
    }
    stream.writeEndElement(); // Transitions

    stream.writeEndElement(); // FSM
    stream.writeEndDocument();

    file->flush();
    file->close();
}

void Fsm::parseXML(const QString& path)
{
    QFile* file = new QFile(path);
    file->open(QIODevice::ReadOnly);

    QDomDocument* document = new QDomDocument();
    if (!document->setContent(file))
    {
        qDebug() << "(Fsm:) Unable to open document as a correct XML file";
        return;
    }

    QDomElement rootNode = document->documentElement();

    if (rootNode.tagName() != "FSM")
    {
        qDebug() << "(Fsm:) Unexpected token found while parsing XML file: expected \"FSM\", got " << rootNode.tagName();
        return;
    }

    QDomNodeList fsmNodes = rootNode.childNodes();

    // First parse signals
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "Signals")
        {
            parseSignals(currentElement);
            break;
        }
    }

    // Then parse states
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "States")
        {
            parseStates(currentElement);
            break;
        }
    }

    // Finally parse transitions
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "Transitions")
        {
            parseTransitions(currentElement);
            break;
        }
    }
}

void Fsm::parseSignals(QDomElement element)
{
    QDomNodeList signalNodes = element.childNodes();

    for (int i = 0 ; i < signalNodes.count() ; i++)
    {
        QDomElement currentElement = signalNodes.at(i).toElement();

        if (currentElement.tagName() == "Input")
        {
            addInput(currentElement.attribute("Name"));
        }
        else if (currentElement.tagName() == "Output")
        {
            addOutput(currentElement.attribute("Name"));
        }
        else if (currentElement.tagName() == "Variable")
        {
            addLocalVariable(currentElement.attribute("Name"));
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected token encountered while parsing signal list: " << currentElement.tagName();
            break;
        }
    }
}

void Fsm::parseStates(QDomElement element)
{
    QDomNodeList stateNodes = element.childNodes();

    for (int i = 0 ; i < stateNodes.count() ; i++)
    {
        QDomElement currentElement = stateNodes.at(i).toElement();

        if (currentElement.tagName() == "State")
        {
            qreal x = currentElement.attribute("X").toDouble();
            qreal y = currentElement.attribute("Y").toDouble();

            FsmState* state = new FsmState(this, currentElement.attribute("Name"));
            state->position = QPointF(x, y);

            if (currentElement.attribute("IsInitial").count() != 0)
                state->setInitial();


            QList<LogicVariable*>* actions = parseActions(currentElement);
            state->setActions(actions);
            delete actions;
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected token encountered while parsing state list: expected \"State\", got: " << currentElement.tagName();
            break;
        }
    }
}

void Fsm::parseTransitions(QDomElement element)
{
    QDomNodeList transitionNodes = element.childNodes();

    for (int i = 0 ; i < transitionNodes.count() ; i++)
    {
        QDomElement currentElement = transitionNodes.at(i).toElement();

        if (currentElement.tagName() == "Transition")
        {
            QString sourceName = currentElement.attribute("Source");
            QString targetName = currentElement.attribute("Target");

            FsmState* source = nullptr;
            FsmState* target = nullptr;
            foreach (FsmState* state, states)
            {
                if (state->getName() == sourceName)
                    source = state;

                if (state->getName() == targetName)
                    target = state;
            }

            FsmTransition* transition = new FsmTransition(this, source, target, nullptr);

            QDomNodeList childNodes = currentElement.childNodes();

            for (int i = 0 ; i < childNodes.count() ; i++)
            {
                QDomElement currentElement = childNodes.at(i).toElement();

                if (currentElement.tagName() == "Condition")
                {
                    transition->setCondition(parseEquation(currentElement));
                }
                else if (currentElement.tagName() == "Actions")
                {
                    QList<LogicVariable*>* actions = parseActions(currentElement);
                    transition->setActions(actions);
                    delete actions;
                }
                else
                {
                    qDebug() << "(Fsm:) Unexpected type encountered while parsing transition list: " << currentElement.tagName();
                    break;
                }


            }
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected token encountered while parsing state list: expected \"Transition\", got: " << currentElement.tagName();
            break;
        }
    }
}

QList<LogicVariable*>* Fsm::parseActions(QDomElement element) const
{
    QList<LogicVariable*>* list = new QList<LogicVariable*>();

    QDomNodeList childNodes = element.childNodes();

    for (int i = 0 ; i < childNodes.count() ; i++)
    {
        QDomElement currentElement = childNodes.at(i).toElement();

        if (currentElement.attribute("Type") == "Output")
            list->append(outputs[currentElement.attribute("Name")]);
        else if (currentElement.attribute("Type") == "Variable")
            list->append(localVariables[currentElement.attribute("Name")]);
        else
        {
            qDebug() << "(Fsm:) Unexpected action type encountered while parsing action list: " << currentElement.attribute("Type");
            break;
        }
    }

    return list;
}

LogicVariable* Fsm::parseEquation(QDomElement element) const
{
    LogicVariable* equation = nullptr;

    QDomNodeList equationNodes = element.childNodes();

    for (int i = 0 ; i < equationNodes.count() ; i++)
    {
        QDomElement currentElement = equationNodes.at(i).toElement();

        if (currentElement.tagName() == "LogicVariable")
        {
            if (currentElement.attribute("Name") == "0")
                equation = LogicVariable::constant0;
            else if (currentElement.attribute("Name") == "1")
                equation = LogicVariable::constant1;
            else
            {
                foreach (LogicVariable* var, getReadableVariables())
                {
                    if (var->getName() == currentElement.attribute("Name"))
                        equation = var;
                }
            }

        }
        else if (currentElement.tagName() == "LogicEquation")
        {
            LogicEquation::nature equationType;

            if (currentElement.attribute("Nature") == "not")
                equationType = LogicEquation::nature::notOp;
            else if (currentElement.attribute("Nature") == "and")
                equationType = LogicEquation::nature::andOp;
            else if (currentElement.attribute("Nature") == "or")
                equationType = LogicEquation::nature::orOp;
            else if (currentElement.attribute("Nature") == "xor")
                equationType = LogicEquation::nature::xorOp;
            else if (currentElement.attribute("Nature") == "nand")
                equationType = LogicEquation::nature::nandOp;
            else if (currentElement.attribute("Nature") == "nor")
                equationType = LogicEquation::nature::norOp;
            else if (currentElement.attribute("Nature") == "xnor")
                equationType = LogicEquation::nature::xnorOp;
            else
            {
                qDebug() << "(Fsm:) Unexpected equation nature encountered while parsing logic equation: " << currentElement.attribute("Nature");
                return nullptr;
            }

            QDomNodeList childNodes = currentElement.childNodes();

            QMap<int, LogicVariable *> operands;

            for (int i = 0 ; i < childNodes.count() ; i++)
            {
                QDomElement currentElement = childNodes.at(i).toElement();

                if (currentElement.tagName() == "Operand")
                {
                    operands[currentElement.attribute("Number").toInt()] = parseEquation(currentElement);
                }
                else
                {
                    qDebug() << "(Fsm:) Unexpected operand nature encountered while parsing logic equation: " << currentElement.tagName();
                    break;
                }
            }

            // Create equation
            equation = new LogicEquation(equationType, operands);

        }
        else
        {
            qDebug() << "(Fsm:) Unexpected action type encountered while parsing equation: " << currentElement.tagName();
            break;
        }
    }

    return equation;
}

void Fsm::writeLogicEquation(QXmlStreamWriter& stream, LogicVariable* equation) const
{
    LogicEquation* complexEquation = dynamic_cast<LogicEquation*> (equation);

    if (complexEquation != nullptr)
    {
        stream.writeStartElement("LogicEquation");
        switch (complexEquation->getFunction())
        {
        case LogicEquation::nature::andOp:
            stream.writeAttribute("Nature", "and");
            break;
        case LogicEquation::nature::nandOp:
            stream.writeAttribute("Nature", "nand");
            break;
        case LogicEquation::nature::norOp:
            stream.writeAttribute("Nature", "nor");
            break;
        case LogicEquation::nature::notOp:
            stream.writeAttribute("Nature", "not");
            break;
        case LogicEquation::nature::orOp:
            stream.writeAttribute("Nature", "or");
            break;
        case LogicEquation::nature::xnorOp:
            stream.writeAttribute("Nature", "xnor");
            break;
        case LogicEquation::nature::xorOp:
            stream.writeAttribute("Nature", "xor");
            break;
        }

        for (int i = 0 ; i < complexEquation->getSize() ; i++)
        {
            stream.writeStartElement("Operand");
            stream.writeAttribute("Number", QString::number(i));
            writeLogicEquation(stream, complexEquation->getOperand(i));
            stream.writeEndElement(); // Operand
        }
    }
    else
    {
        stream.writeStartElement("LogicVariable");
        stream.writeAttribute("Name", equation->getName());
    }

    stream.writeEndElement(); // LogicEquation | LogicVariable
}
