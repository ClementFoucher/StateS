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
#include "fsm.h"

// Qt classes
#include <QFile>

// Debug
#include <QDebug>

// StateS classes
#include "equation.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "input.h"
#include "output.h"
#include "fsmgraphicalstate.h"


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
    foreach (Signal* var, getReadableSignals())
    {
        emit var->signalStateChangedEvent();
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
            emit initialState->elementConfigurationChangedEvent();

        if (previousInitialState != nullptr)
            emit previousInitialState->elementConfigurationChangedEvent();
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
    foreach (Signal* var, getAllSignals())
    {
        // Type
        if (dynamic_cast<Input*>(var) != nullptr)
            stream.writeStartElement("Input");
        else if (dynamic_cast<Output*>(var) != nullptr)
            stream.writeStartElement("Output");
        else if (var->getIsConstant())
            stream.writeStartElement("Constant");
        else
            stream.writeStartElement("Variable");

        // Name
        stream.writeAttribute("Name", var->getName());

        // Size
        stream.writeAttribute("Size", QString::number(var->getSize()));

        // Initial value (except for outputs)
        if (dynamic_cast<Output*>(var) == nullptr)
            stream.writeAttribute("Initial_value", var->getInitialValue().toString());

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
        writeActions(stream, state);

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
        if (transition->getCondition() != nullptr)
        {
            stream.writeStartElement("Condition");
            writeLogicEquation(stream, transition->getCondition());
            stream.writeEndElement(); // Condition
        }

        // Actions
        writeActions(stream, transition);

        stream.writeEndElement(); // Transition
    }
    stream.writeEndElement(); // Transitions

    stream.writeEndElement(); // FSM
    stream.writeEndDocument();

    file->flush();
    file->close();
    delete file;
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
            addSignal(signal_types::Input, currentElement.attribute("Name"));
        }
        else if (currentElement.tagName() == "Output")
        {
            addSignal(signal_types::Output, currentElement.attribute("Name"));
        }
        else if (currentElement.tagName() == "Variable")
        {
            addSignal(signal_types::LocalVariable, currentElement.attribute("Name"));
        }
        else if (currentElement.tagName() == "Constant")
        {
            addSignal(signal_types::Constant, currentElement.attribute("Name"));
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected token encountered while parsing signal list: " << currentElement.tagName();
            break;
        }

        // Set size
        uint size = (uint)currentElement.attribute("Size").toInt();
        if (size != 1)
        {
            resizeSignal(currentElement.attribute("Name"), size);
        }

        // Set initial value
        if (currentElement.tagName() != "Output")
        {
            LogicValue initialValue = LogicValue::fromString(currentElement.attribute("Initial_value"));
            changeSignalInitialValue(currentElement.attribute("Name"), initialValue);
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

            QDomNodeList childNodes = currentElement.childNodes();

            for (int i = 0 ; i < childNodes.count() ; i++)
            {
                QDomElement currentElement = childNodes.at(i).toElement();

                if (currentElement.tagName() == "Actions")
                {
                    parseActions(currentElement, state);
                }
                else
                {
                    qDebug() << "(Fsm:) Unexpected type encountered while parsing state list: " << currentElement.tagName();
                    break;
                }
            }
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
                    parseActions(currentElement, transition);
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

void Fsm::parseActions(QDomElement element, MachineActuatorComponent* component) const
{
    QDomNodeList childNodes = element.childNodes();

    for (int i = 0 ; i < childNodes.count() ; i++)
    {
        QDomElement currentElement = childNodes.at(i).toElement();
        Signal* signal = nullptr;

        if (currentElement.attribute("Signal_Type") == "Output")
            signal = outputs[currentElement.attribute("Name")];
        else if (currentElement.attribute("Signal_Type") == "Variable")
            signal = localVariables[currentElement.attribute("Name")];
        else
        {
            qDebug() << "(Fsm:) Unexpected signal type encountered while parsing action list: " << currentElement.attribute("Signal_Type");
            break;
        }

        component->addAction(signal);

        QString actionType = currentElement.attribute("Action_Type");

        if (actionType == "Pulse")
        {
            component->setActionType(signal, MachineActuatorComponent::action_types::pulse);
        }
        else if (actionType == "Set")
        {
            component->setActionType(signal, MachineActuatorComponent::action_types::set);
        }
        else if (actionType == "Reset")
        {
            component->setActionType(signal, MachineActuatorComponent::action_types::reset);
        }
        else if (actionType == "Assign")
        {
            component->setActionType(signal, MachineActuatorComponent::action_types::assign);
            component->setActionValue(signal, LogicValue::fromString(currentElement.attribute("Action_Value")));
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected action type encountered while parsing action list: " << currentElement.attribute("Action_Type");
            break;
        }
    }
}

Signal* Fsm::parseEquation(QDomElement element) const
{
    Signal* equation = nullptr;

    QDomNodeList equationNodes = element.childNodes();

    for (int i = 0 ; i < equationNodes.count() ; i++)
    {
        QDomElement currentElement = equationNodes.at(i).toElement();

        if (currentElement.tagName() == "LogicVariable")
        {
            foreach (Signal* var, getReadableSignals())
            {
                if (var->getName() == currentElement.attribute("Name"))
                    equation = var;
            }
        }
        else if (currentElement.tagName() == "LogicEquation")
        {
            Equation::nature equationType;

            if (currentElement.attribute("Nature") == "not")
                equationType = Equation::nature::notOp;
            else if (currentElement.attribute("Nature") == "and")
                equationType = Equation::nature::andOp;
            else if (currentElement.attribute("Nature") == "or")
                equationType = Equation::nature::orOp;
            else if (currentElement.attribute("Nature") == "xor")
                equationType = Equation::nature::xorOp;
            else if (currentElement.attribute("Nature") == "nand")
                equationType = Equation::nature::nandOp;
            else if (currentElement.attribute("Nature") == "nor")
                equationType = Equation::nature::norOp;
            else if (currentElement.attribute("Nature") == "xnor")
                equationType = Equation::nature::xnorOp;
            else if (currentElement.attribute("Nature") == "equals")
                equationType = Equation::nature::equalOp;
            else if (currentElement.attribute("Nature") == "differs")
                equationType = Equation::nature::diffOp;
            else
            {
                qDebug() << "(Fsm:) Unexpected equation nature encountered while parsing logic equation: " << currentElement.attribute("Nature");
                return nullptr;
            }

            QDomNodeList childNodes = currentElement.childNodes();

            QMap<int, Signal*> operandsMap;

            for (int i = 0 ; i < childNodes.count() ; i++)
            {
                QDomElement currentElement = childNodes.at(i).toElement();

                if (currentElement.tagName() == "Operand")
                {
                    operandsMap[currentElement.attribute("Number").toInt()] = parseEquation(currentElement);
                }
                else
                {
                    qDebug() << "(Fsm:) Unexpected operand nature encountered while parsing logic equation: " << currentElement.tagName();
                    break;
                }
            }

            // Create equation
            QVector<Signal*> operands;

            for (int i = 0 ; i < operandsMap.count() ; i++)
            {
                operands.append(operandsMap[i]);
            }

            equation = new Equation(equationType, operands);

        }
        else
        {
            qDebug() << "(Fsm:) Unexpected action type encountered while parsing equation: " << currentElement.tagName();
            break;
        }
    }

    return equation;
}

void Fsm::writeLogicEquation(QXmlStreamWriter& stream, Signal* equation) const
{
    Equation* complexEquation = dynamic_cast<Equation*> (equation);

    if (complexEquation != nullptr)
    {
        stream.writeStartElement("LogicEquation");
        switch (complexEquation->getFunction())
        {
        case Equation::nature::andOp:
            stream.writeAttribute("Nature", "and");
            break;
        case Equation::nature::nandOp:
            stream.writeAttribute("Nature", "nand");
            break;
        case Equation::nature::norOp:
            stream.writeAttribute("Nature", "nor");
            break;
        case Equation::nature::notOp:
            stream.writeAttribute("Nature", "not");
            break;
        case Equation::nature::orOp:
            stream.writeAttribute("Nature", "or");
            break;
        case Equation::nature::xnorOp:
            stream.writeAttribute("Nature", "xnor");
            break;
        case Equation::nature::xorOp:
            stream.writeAttribute("Nature", "xor");
            break;
        case Equation::nature::equalOp:
            stream.writeAttribute("Nature", "equals");
            break;
        case Equation::nature::diffOp:
            stream.writeAttribute("Nature", "differs");
        case Equation::nature::identity:
            qDebug() << "(Fsm) Error! Trying to write identity to save file, while this should be an internal type. Ignored.";
            break;
        }

        for (uint i = 0 ; i < complexEquation->getOperandCount() ; i++)
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

void Fsm::writeActions(QXmlStreamWriter& stream, MachineActuatorComponent* component) const
{
    if (component->getActions().count() != 0)
    {
        stream.writeStartElement("Actions");
        foreach (Signal* action, component->getActions())
        {
            stream.writeStartElement("Action");

            if ((dynamic_cast<Output*> (action)) != nullptr)
                stream.writeAttribute("Signal_Type", "Output");
            else
                stream.writeAttribute("Signal_Type", "Variable");

            stream.writeAttribute("Name", action->getName());

            switch(component->getActionType(action))
            {
            case MachineActuatorComponent::action_types::pulse:
                stream.writeAttribute("Action_Type", "Pulse");
                break;
            case MachineActuatorComponent::action_types::set:
                stream.writeAttribute("Action_Type", "Set");
                break;
            case MachineActuatorComponent::action_types::reset:
                stream.writeAttribute("Action_Type", "Reset");
                break;
            case MachineActuatorComponent::action_types::assign:
                stream.writeAttribute("Action_Type", "Assign");
                stream.writeAttribute("Action_Value", component->getActionValue(action).toString());
                break;
            }

            stream.writeEndElement(); // Action
        }
        stream.writeEndElement(); // Actions
    }
}
