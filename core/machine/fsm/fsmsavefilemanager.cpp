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
#include "fsmsavefilemanager.h"

// Qt classes
#include <QFileInfo>
#include <QDir>
#include <QXmlStreamWriter>
#include <QDomElement>

// Debug
#include <QDebug>

// StateS classes
#include "fsm.h"
#include "signal.h"
#include "fsmstate.h"
#include "fsmgraphicstate.h"
#include "fsmtransition.h"
#include "equation.h"
#include "input.h"
#include "output.h"
#include "constant.h"
#include "statesexception.h"


void FsmSaveFileManager::writeToFile(shared_ptr<Fsm> machine, const QString& filePath) // Throws StatesException
{
    QFileInfo fileInfo(filePath);
    if ( (fileInfo.exists()) && (!fileInfo.isWritable()) ) // Replace existing file
    {
        throw StatesException("FsmSaveFileManager", unable_to_replace, "Unable to replace existing file");
    }
    else if ( !fileInfo.absoluteDir().exists() )
    {
        throw StatesException("FsmSaveFileManager", unkown_directory, "Directory doesn't exist");
    }

    unique_ptr<QFile> file = unique_ptr<QFile>(new QFile(filePath));
    bool fileOpened = file->open(QIODevice::WriteOnly);
    if (fileOpened == false)
    {
        throw StatesException("FsmSaveFileManager", unable_to_open, "Unable to open file");
    }

    QXmlStreamWriter stream(file.get());

    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("FSM");
    stream.writeAttribute("Name", machine->getName());

    FsmSaveFileManager::writeSignals(stream, machine);
    FsmSaveFileManager::writeStates(stream, machine);
    FsmSaveFileManager::writeTransitions(stream, machine);

    stream.writeEndElement(); // End FSM element
    stream.writeEndDocument();

    file->close();

    machine->setUnsavedState(false);
}

void FsmSaveFileManager::writeSignals(QXmlStreamWriter& stream, shared_ptr<Fsm> machine)
{
    stream.writeStartElement("Signals");

    foreach (shared_ptr<Signal> var, machine->getAllSignals())
    {
        // Type
        if (dynamic_pointer_cast<Input>(var) != nullptr)
            stream.writeStartElement("Input");
        else if (dynamic_pointer_cast<Output>(var) != nullptr)
            stream.writeStartElement("Output");
        else if (dynamic_pointer_cast<Constant>(var) != nullptr)
            stream.writeStartElement("Constant");
        else
            stream.writeStartElement("Variable");

        // Name
        stream.writeAttribute("Name", var->getName());

        // Size
        stream.writeAttribute("Size", QString::number(var->getSize()));

        // Initial value (except for outputs)
        if (dynamic_pointer_cast<Output>(var) == nullptr)
            stream.writeAttribute("Initial_value", var->getInitialValue().toString());

        stream.writeEndElement();
    }

    stream.writeEndElement();
}

void FsmSaveFileManager::writeStates(QXmlStreamWriter& stream, shared_ptr<Fsm> machine)
{
    stream.writeStartElement("States");

    foreach (shared_ptr<FsmState> state, machine->getStates())
    {
        stream.writeStartElement("State");

        // Name
        stream.writeAttribute("Name", state->getName());

        // Initial
        if (state->isInitial())
            stream.writeAttribute("IsInitial", "true");

        // Position
        stream.writeAttribute("X", QString::number(state->getGraphicRepresentation()->scenePos().x()));
        stream.writeAttribute("Y", QString::number(state->getGraphicRepresentation()->scenePos().y()));

        // Actions
        FsmSaveFileManager::writeActions(stream, state);

        stream.writeEndElement();
    }

    stream.writeEndElement();
}

void FsmSaveFileManager::writeTransitions(QXmlStreamWriter& stream, shared_ptr<Fsm> machine)
{
    stream.writeStartElement("Transitions");

    foreach (shared_ptr<FsmTransition> transition, machine->getTransitions())
    {
        stream.writeStartElement("Transition");

        stream.writeAttribute("Source", transition->getSource()->getName());
        stream.writeAttribute("Target", transition->getTarget()->getName());

        // Deal with equations
        if (transition->getCondition() != nullptr)
        {
            stream.writeStartElement("Condition");
            FsmSaveFileManager::writeLogicEquation(stream, transition->getCondition());
            stream.writeEndElement(); // Condition
        }

        // Actions
        FsmSaveFileManager::writeActions(stream, transition);

        stream.writeEndElement();
    }

    stream.writeEndElement();
}

void FsmSaveFileManager::writeLogicEquation(QXmlStreamWriter& stream, shared_ptr<Signal> equation)
{
    shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (equation);

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
            break;
        case Equation::nature::extractOp:
            stream.writeAttribute("Nature", "extract");
            stream.writeAttribute("Param1", QString::number(complexEquation->getParam1()));
            stream.writeAttribute("Param2", QString::number(complexEquation->getParam2()));
            break;
        case Equation::nature::concatOp:
            stream.writeAttribute("Nature", "concatenate");
            break;
        case Equation::nature::constant:
            stream.writeAttribute("Nature", "constant");
            stream.writeAttribute("Value", complexEquation->getCurrentValue().toString());
            break;
        case Equation::nature::identity:
            // Should not happen
            break;
        }

        for (uint i = 0 ; i < complexEquation->getOperandCount() ; i++)
        {
            shared_ptr<Signal> operand = complexEquation->getOperand(i); // Throws StatesException - Constrained by operand count - ignored
            if (operand != nullptr)
            {
                stream.writeStartElement("Operand");
                stream.writeAttribute("Number", QString::number(i));
                writeLogicEquation(stream, operand);
                stream.writeEndElement(); // Operand
            }
        }
    }
    else
    {
        stream.writeStartElement("LogicVariable");
        stream.writeAttribute("Name", equation->getName());
    }

    stream.writeEndElement(); // LogicEquation | LogicVariable
}

void FsmSaveFileManager::writeActions(QXmlStreamWriter& stream, shared_ptr<MachineActuatorComponent> component)
{
    if (component->getActions().count() != 0)
    {
        stream.writeStartElement("Actions");
        foreach (shared_ptr<Signal> action, component->getActions())
        {
            stream.writeStartElement("Action");

            if ((dynamic_pointer_cast<Output> (action)) != nullptr)
                stream.writeAttribute("Signal_Type", "Output");
            else
                stream.writeAttribute("Signal_Type", "Variable");

            stream.writeAttribute("Name", action->getName());

            switch(component->getActionType(action))
            {
            case MachineActuatorComponent::action_types::activeOnState:
                stream.writeAttribute("Action_Type", "ActiveOnState");
                break;
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
                break;
            }

            if (!component->getActionValue(action).isNull())
                stream.writeAttribute("Action_Value", component->getActionValue(action).toString());
            if (component->getActionParam1(action) != -1)
                stream.writeAttribute("Param1", QString::number(component->getActionParam1(action)));
            if (component->getActionParam2(action) != -1)
                stream.writeAttribute("Param2", QString::number(component->getActionParam2(action)));

            stream.writeEndElement(); // Action
        }
        stream.writeEndElement(); // Actions
    }
}



shared_ptr<Fsm> FsmSaveFileManager::loadFromFile(const QString& filePath) // Throws StatesException
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists())
    {
       throw StatesException("FsmSaveFileManager", inexistant_file, "Can't find file");
    }
    else if (!fileInfo.isReadable())
    {
        throw StatesException("FsmSaveFileManager", permission_denied, "Can't read file");
    }

    unique_ptr<QFile> file = unique_ptr<QFile>(new QFile(filePath));
    bool fileOpened = file->open(QIODevice::ReadOnly);
    if (fileOpened == false)
    {
        throw StatesException("FsmSaveFileManager", unable_to_open, "Unable to open file");
    }

    QDomDocument* document = new QDomDocument();
    if (!document->setContent(file.get()))
    {
        throw StatesException("FsmSaveFileManager", wrong_xml, "Unable to open document as a correct XML file");
    }

    QDomElement rootNode = document->documentElement();
    if (rootNode.tagName() != "FSM")
    {
        throw StatesException("FsmSaveFileManager", wrong_xml, "Unexpected token found while parsing XML file: expected \"FSM\", got " + rootNode.tagName());
    }

    shared_ptr<Fsm> machine(new Fsm());

    QString machineName = rootNode.attribute("Name");
    if (machineName == QString::null)
    {
        machineName = file->fileName();
        machineName = machineName.section("/", -1, -1);             // Extract file name from path
        machineName.remove("." + machineName.section(".", -1, -1)); // Remove extension
    }
    if (machineName == QString::null) // In case we still have no name (file with no file name?)
    {
        machineName = tr("Machine");
    }
    machine->setName(machineName);

    QDomNodeList fsmNodes = rootNode.childNodes();

    // First parse signals
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "Signals")
        {
            parseSignals(currentElement, machine);
            break;
        }
    }

    // Then parse states
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "States")
        {
            parseStates(currentElement, machine);
            break;
        }
    }

    // Finally parse transitions
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "Transitions")
        {
            parseTransitions(currentElement, machine);
            break;
        }
    }

    file->close();

    machine->setUnsavedState(false);

    return machine;
}


void FsmSaveFileManager::parseSignals(QDomElement element, shared_ptr<Fsm> machine)
{
    QDomNodeList signalNodes = element.childNodes();
    QString currentElementName;

    for (int i = 0 ; i < signalNodes.count() ; i++)
    {
        QDomElement currentElement = signalNodes.at(i).toElement();
        currentElementName = currentElement.attribute("Name");

        shared_ptr<Signal> signal;
        if (currentElement.tagName() == "Input")
        {
            signal = machine->addSignal(Fsm::signal_type::Input, currentElementName);
        }
        else if (currentElement.tagName() == "Output")
        {
            signal = machine->addSignal(Fsm::signal_type::Output, currentElementName);
        }
        else if (currentElement.tagName() == "Variable")
        {
            signal = machine->addSignal(Fsm::signal_type::LocalVariable, currentElementName);
        }
        else if (currentElement.tagName() == "Constant")
        {
            signal = machine->addSignal(Fsm::signal_type::Constant, currentElementName);
        }
        else
        {
            qDebug() << "(FsmSaveFileManager:) Unexpected signal type encountered while parsing signal list: " << currentElement.tagName() << ". Signal name was: " << currentElementName;
            qDebug() << "Token ignored";
            continue;
        }

        // Set size
        uint size = (uint)currentElement.attribute("Size").toInt();
        if (size != 1)
        {
            try
            {
                machine->resizeSignal(currentElementName, size); // Throws StatesException
            }
            catch (const StatesException& e)
            {
                // TODO: check which class is the actual source of the exception + adpat error text
                qDebug() << "(FsmSaveFileManager:) Error in size of signal " << currentElementName << ".";
                qDebug() << "The exception was raised by " << e.getSourceClass() << " and the concern is: \"" << e.what() << "\".";
                qDebug() << "Signal size ignored and defaulted to " << QString::number(signal->getSize()) << ".";
            }
        }

        // Set initial value
        if (currentElement.tagName() != "Output")
        {
            try
            {
                LogicValue initialValue = LogicValue::fromString(currentElement.attribute("Initial_value")); // Throws StatesException

                machine->changeSignalInitialValue(currentElementName, initialValue); // Throws StatesException
            }
            catch (const StatesException& e)
            {
                // TODO: check which class is the actual source of the exception + adpat error text
                qDebug() << "(FsmSaveFileManager:) Error in initial value of signal " << currentElementName << ".";
                qDebug() << "The exception was raised by " << e.getSourceClass() << " and the concern is: \"" << e.what() << "\".";
                qDebug() << "Initial value ignored and defaulted to " << signal->getInitialValue().toString() << ".";
            }
        }
    }
}

void FsmSaveFileManager::parseStates(QDomElement element, shared_ptr<Fsm> machine)
{
    QDomNodeList stateNodes = element.childNodes();

    for (int i = 0 ; i < stateNodes.count() ; i++)
    {
        QDomElement currentElement = stateNodes.at(i).toElement();

        if (currentElement.tagName() == "State")
        {
            qreal x = currentElement.attribute("X").toDouble();
            qreal y = currentElement.attribute("Y").toDouble();

            shared_ptr<FsmState> state = machine->addState(currentElement.attribute("Name"));
            state->position = QPointF(x, y);

            if (currentElement.attribute("IsInitial").count() != 0)
                state->setInitial();

            QDomNodeList childNodes = currentElement.childNodes();

            for (int i = 0 ; i < childNodes.count() ; i++)
            {
                QDomElement currentElement = childNodes.at(i).toElement();

                if (currentElement.tagName() == "Actions")
                {
                    parseActions(currentElement, state, machine);
                }
                else
                {
                    qDebug() << "(Fsm:) Unexpected token encountered while parsing state list: " << currentElement.tagName();
                    qDebug() << "Token ignored";
                    continue;
                }
            }
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected token encountered while parsing state list: expected \"State\", got: " << currentElement.tagName();
            qDebug() << "Token ignored";
            continue;
        }
    }
}

void FsmSaveFileManager::parseTransitions(QDomElement element, shared_ptr<Fsm> machine)
{
    QDomNodeList transitionNodes = element.childNodes();

    for (int i = 0 ; i < transitionNodes.count() ; i++)
    {
        QDomElement currentElement = transitionNodes.at(i).toElement();

        if (currentElement.tagName() == "Transition")
        {
            QString sourceName = currentElement.attribute("Source");
            QString targetName = currentElement.attribute("Target");

            shared_ptr<FsmState> source = machine->getStateByName(sourceName);
            shared_ptr<FsmState> target = machine->getStateByName(targetName);

            shared_ptr<FsmTransition> transition(new FsmTransition(machine, source, target, nullptr));
            source->addOutgoingTransition(transition);
            target->addIncomingTransition(transition);

            QDomNodeList childNodes = currentElement.childNodes();

            for (int i = 0 ; i < childNodes.count() ; i++)
            {
                QDomElement currentElement = childNodes.at(i).toElement();

                if (currentElement.tagName() == "Condition")
                {
                    transition->setCondition(parseEquation(currentElement, machine));
                }
                else if (currentElement.tagName() == "Actions")
                {
                    parseActions(currentElement, transition, machine);
                }
                else
                {
                    qDebug() << "(Fsm:) Unexpected token encountered while parsing transition list: " << currentElement.tagName();
                    qDebug() << "Token ignored";
                    continue;
                }
            }
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected token encountered while parsing state list: expected \"Transition\", got: " << currentElement.tagName();
            qDebug() << "Token ignored";
            continue;
        }
    }
}

void FsmSaveFileManager::parseActions(QDomElement element, shared_ptr<MachineActuatorComponent> component, shared_ptr<Fsm> machine)
{
    QDomNodeList childNodes = element.childNodes();

    for (int i = 0 ; i < childNodes.count() ; i++)
    {
        QDomElement currentElement = childNodes.at(i).toElement();
        shared_ptr<Signal> signal;

        foreach (shared_ptr<Signal> var, machine->getWrittableSignals())
        {
            if (var->getName() == currentElement.attribute("Name"))
                signal = var;
        }

        if (signal == nullptr)
        {
            qDebug() << "(Fsm:) Unexpected signal encountered while parsing action list: " << currentElement.attribute("Signal_Type");
            break;
        }

        component->addAction(signal);

        QString actionType = currentElement.attribute("Action_Type");

        if (actionType == "Pulse")
        {
            component->setActionType(signal, MachineActuatorComponent::action_types::pulse);
        }
        else if (actionType == "ActiveOnState")
        {
            component->setActionType(signal, MachineActuatorComponent::action_types::activeOnState);
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
        }
        else
        {
            qDebug() << "(Fsm:) Unexpected action type encountered while parsing action list: " << currentElement.attribute("Action_Type");
            break;
        }


        QString sparam1 = currentElement.attribute("Param1");
        QString sparam2 = currentElement.attribute("Param2");
        QString sactval = currentElement.attribute("Action_Value");

        int param1;
        int param2;
        LogicValue actionValue;
        bool setActionValue = false;

        if (! sparam1.isEmpty())
        {
            setActionValue = true;
            param1 = sparam1.toInt();
        }
        else
            param1 = -1;

        if (! sparam2.isEmpty())
        {
            setActionValue = true;
            param2 = sparam2.toInt();
        }
        else
            param2 = -1;

        if(! sactval.isEmpty())
        {
            setActionValue = true;
            try
            {
                actionValue = LogicValue::fromString(sactval); // Throws StatesException
            }
            catch (const StatesException& e)
            {
                if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
                {
                    uint avsize;
                    if ( (param1 != -1) && (param2 == -1) )
                        avsize = 1;
                    else if ( (param1 != -1) && (param2 != -1) )
                        avsize = param1 - param2 + 1;
                    else
                        avsize = 1; // TODO: determine actual size

                    actionValue = LogicValue::getValue0(avsize);

                    qDebug() << "(FsmSaveFileManager:) Error in action value.";
                    qDebug() << "Value ignored and set to " << actionValue.toString();
                }
                else
                    throw;
            }
        }
        else
            actionValue = LogicValue::getNullValue();

        if (setActionValue)
            component->setActionValue(signal, actionValue, param1, param2);
    }
}

shared_ptr<Signal> FsmSaveFileManager::parseEquation(QDomElement element, shared_ptr<Fsm> machine)
{
    shared_ptr<Signal> equation;

    QDomNodeList equationNodes = element.childNodes();

    for (int i = 0 ; i < equationNodes.count() ; i++)
    {
        QDomElement currentElement = equationNodes.at(i).toElement();

        if (currentElement.tagName() == "LogicVariable")
        {
            foreach (shared_ptr<Signal> var, machine->getReadableSignals())
            {
                if (var->getName() == currentElement.attribute("Name"))
                {
                    equation = var;
                    break;
                }
            }
        }
        else if (currentElement.tagName() == "LogicEquation")
        {
            Equation::nature equationType;
            int param1 = -1;
            int param2 = -1;
            LogicValue constantValue;

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
            else if (currentElement.attribute("Nature") == "concatenate")
                equationType = Equation::nature::concatOp;
            else if (currentElement.attribute("Nature") == "extract")
            {
                equationType = Equation::nature::extractOp;

                param1 = currentElement.attribute("Param1").toInt();
                param2 = currentElement.attribute("Param2").toInt();
            }
            else if (currentElement.attribute("Nature") == "constant")
            {
                equationType = Equation::nature::constant;

                try
                {
                    constantValue = LogicValue::fromString(currentElement.attribute("Value")); // Throws StatesException
                }
                catch (const StatesException& e)
                {
                    if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
                    {
                        uint constantsize;
                        if ( (param1 != -1) && (param2 == -1) )
                            constantsize = 1;
                        else if ( (param1 != -1) && (param2 != -1) )
                            constantsize = param1 - param2 + 1;
                        else
                            constantsize = 1; // TODO: determine actual size

                        constantValue = LogicValue::getValue0(constantsize);

                        qDebug() << "(FsmSaveFileManager:) Warning: Error in constant value.";
                        qDebug() << "Value ignored and set to " << constantValue.toString();
                    }
                    else
                        throw;
                }
            }
            else
            {
                qDebug() << "(FsmSaveFileManager:) Warning: Unexpected equation nature encountered while parsing logic equation: " << currentElement.attribute("Nature");
                qDebug() << "Token ignored. Will retry with other tokens if existing.";
                continue;
            }

            QDomNodeList childNodes = currentElement.childNodes();

            QMap<int, shared_ptr<Signal>> operandsMap;

            for (int i = 0 ; i < childNodes.count() ; i++)
            {
                QDomElement currentElement = childNodes.at(i).toElement();

                if (currentElement.tagName() == "Operand")
                {
                    operandsMap[currentElement.attribute("Number").toInt()] = parseEquation(currentElement, machine);
                }
                else
                {
                    qDebug() << "(FsmSaveFileManager:) Unexpected token encountered while parsing logic equation: expected \"Operand\", got " << currentElement.tagName();
                    qDebug() << "Token ignored. Will retry with other tokens if existing.";
                    continue;
                }
            }

            // Create equation
            QVector<shared_ptr<Signal>> operands;

            for (int i = 0 ; i < operandsMap.count() ; i++)
            {
                // If operandMap has empty ranges, accessing them will create a nullptr.
                // Then, on next check, count() has been raised by one, allowing to
                // effectively go till the last element, putting empty operands where
                // there is no value. Got lucky on this behavior.
                operands.append(operandsMap[i]);
            }

            shared_ptr<Equation> newEquation = shared_ptr<Equation>(new Equation(equationType, operands, param1, param2));

            if (equationType == Equation::nature::constant)
                newEquation->setCurrentValue(constantValue); // Throws StatesException - constantValue is built for signal size - ignored

            equation = newEquation;
        }
        else
        {
            qDebug() << "(FsmSaveFileManager:) Unexpected token encountered while parsing equation:";
            qDebug() << "Expected signal (LogicVariable) or equation (LogicEquation), got: " << currentElement.tagName();
            qDebug() << "Token ignored. Will retry with other tokens if existing.";
            continue;
        }
    }

    return equation;
}
