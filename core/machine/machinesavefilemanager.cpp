/*
 * Copyright © 2017 Clément Foucher
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
#include "machinesavefilemanager.h"

// Qt classes
#include <QDomElement>
#include <QFileInfo>
#include <QXmlStreamWriter>
#include <QDir>
#include <QFile>
#include <QMap>

// StateS classes
#include "statesexception.h"
#include "machineconfiguration.h"
#include "machine.h"
#include "signal.h"
#include "constant.h"
#include "input.h"
#include "output.h"
#include "equation.h"
#include "actiononsignal.h"
#include "machineactuatorcomponent.h"


MachineSaveFileManager::MachineSaveFileManager(QObject* parent) :
    QObject(parent)
{
    this->warnings = QList<QString>();
}

void MachineSaveFileManager::writeMachineCommonElements(shared_ptr<Machine> machine, shared_ptr<MachineConfiguration> configuration)
{
    this->writeMachineConfiguration(configuration);
    this->writeMachineSignals(machine);
}

shared_ptr<MachineConfiguration> MachineSaveFileManager::getConfiguration()
{
    return this->configuration;
}

QList<QString> MachineSaveFileManager::getWarnings()
{
    return this->warnings;
}

void MachineSaveFileManager::writeMachineConfiguration(shared_ptr<MachineConfiguration> configuration)
{
    if (configuration != nullptr)
    {
        this->stream->writeStartElement("Configuration");

        this->stream->writeStartElement("Scale");
        this->stream->writeAttribute("Value", QString::number(configuration->zoomLevel));
        this->stream->writeEndElement();

        this->stream->writeStartElement("ViewCentralPoint");
        this->stream->writeAttribute("X", QString::number(configuration->viewCenter.x() + configuration->sceneTranslation.x()));
        this->stream->writeAttribute("Y", QString::number(configuration->viewCenter.y() + configuration->sceneTranslation.y()));
        this->stream->writeEndElement();

        this->stream->writeEndElement();
    }
}

void MachineSaveFileManager::writeMachineSignals(shared_ptr<Machine> machine)
{
    this->stream->writeStartElement("Signals");

    foreach (shared_ptr<Signal> var, machine->getAllSignals())
    {
        // Type
        if (dynamic_pointer_cast<Input>(var) != nullptr)
            this->stream->writeStartElement("Input");
        else if (dynamic_pointer_cast<Output>(var) != nullptr)
            this->stream->writeStartElement("Output");
        else if (dynamic_pointer_cast<Constant>(var) != nullptr)
            this->stream->writeStartElement("Constant");
        else
            this->stream->writeStartElement("Variable");

        // Name
        this->stream->writeAttribute("Name", var->getName());

        // Size
        this->stream->writeAttribute("Size", QString::number(var->getSize()));

        // Initial value (except for outputs)
        if (dynamic_pointer_cast<Output>(var) == nullptr)
            this->stream->writeAttribute("Initial_value", var->getInitialValue().toString());

        this->stream->writeEndElement();
    }

    this->stream->writeEndElement();
}

void MachineSaveFileManager::writeActuatorActions(shared_ptr<MachineActuatorComponent> component)
{
    QList<shared_ptr<ActionOnSignal>> actions = component->getActions();

    if (actions.count() != 0)
    {
        this->stream->writeStartElement("Actions");
        foreach (shared_ptr<ActionOnSignal> action, actions)
        {
            this->stream->writeStartElement("Action");

            if ((dynamic_pointer_cast<Output> (action)) != nullptr)
                this->stream->writeAttribute("Signal_Type", "Output");
            else
                this->stream->writeAttribute("Signal_Type", "Variable");

            this->stream->writeAttribute("Name", action->getSignalActedOn()->getName());

            switch(action->getActionType())
            {
            case ActionOnSignal::action_types::activeOnState:
                this->stream->writeAttribute("Action_Type", "ActiveOnState");
                break;
            case ActionOnSignal::action_types::pulse:
                this->stream->writeAttribute("Action_Type", "Pulse");
                break;
            case ActionOnSignal::action_types::set:
                this->stream->writeAttribute("Action_Type", "Set");
                break;
            case ActionOnSignal::action_types::reset:
                this->stream->writeAttribute("Action_Type", "Reset");
                break;
            case ActionOnSignal::action_types::assign:
                this->stream->writeAttribute("Action_Type", "Assign");
                break;
            }

            if (!action->getActionValue().isNull())
                this->stream->writeAttribute("Action_Value", action->getActionValue().toString());
            if (action->getActionRangeL() != -1)
                this->stream->writeAttribute("RangeL", QString::number(action->getActionRangeL()));
            if (action->getActionRangeR() != -1)
                this->stream->writeAttribute("RangeR", QString::number(action->getActionRangeR()));

            this->stream->writeEndElement(); // Action
        }
        this->stream->writeEndElement(); // Actions
    }
}

void MachineSaveFileManager::writeLogicEquation(shared_ptr<Signal> equation)
{
    shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (equation);

    if (complexEquation != nullptr)
    {
        this->stream->writeStartElement("LogicEquation");
        switch (complexEquation->getFunction())
        {
        case Equation::nature::andOp:
            this->stream->writeAttribute("Nature", "and");
            this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
            break;
        case Equation::nature::nandOp:
            this->stream->writeAttribute("Nature", "nand");
            this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
            break;
        case Equation::nature::norOp:
            this->stream->writeAttribute("Nature", "nor");
            this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
            break;
        case Equation::nature::notOp:
            this->stream->writeAttribute("Nature", "not");
            break;
        case Equation::nature::orOp:
            this->stream->writeAttribute("Nature", "or");
            this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
            break;
        case Equation::nature::xnorOp:
            this->stream->writeAttribute("Nature", "xnor");
            this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
            break;
        case Equation::nature::xorOp:
            this->stream->writeAttribute("Nature", "xor");
            this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
            break;
        case Equation::nature::equalOp:
            this->stream->writeAttribute("Nature", "equals");
            break;
        case Equation::nature::diffOp:
            this->stream->writeAttribute("Nature", "differs");
            break;
        case Equation::nature::extractOp:
            this->stream->writeAttribute("Nature", "extract");
            this->stream->writeAttribute("RangeL", QString::number(complexEquation->getRangeL()));
            this->stream->writeAttribute("RangeR", QString::number(complexEquation->getRangeR()));
            break;
        case Equation::nature::concatOp:
            this->stream->writeAttribute("Nature", "concatenate");
            this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
            break;
        case Equation::nature::constant:
            this->stream->writeAttribute("Nature", "constant");
            this->stream->writeAttribute("Value", complexEquation->getCurrentValue().toString());
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
                this->stream->writeStartElement("Operand");
                this->stream->writeAttribute("Number", QString::number(i));
                this->writeLogicEquation(operand);
                this->stream->writeEndElement(); // Operand
            }
        }
    }
    else
    {
        this->stream->writeStartElement("LogicVariable");
        this->stream->writeAttribute("Name", equation->getName());
    }

    this->stream->writeEndElement(); // LogicEquation | LogicVariable
}

void MachineSaveFileManager::initializeDocumentFromString(const QString& machineXml) // Throws StatesException
{
    this->document = shared_ptr<QDomDocument>(new QDomDocument());
    if (this->document->setContent(machineXml) == false)
    {
        throw StatesException("MachineSaveFileManager", wrong_xml, "Unable to open document as a correct XML file");
    }
}

void MachineSaveFileManager::initializeDocumentFromFile(const QString& filePath) // Throws StatesException
{
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists() == false)
    {
       throw StatesException("MachineSaveFileManager", inexistant_file, "Can't find file");
    }
    else if (fileInfo.isReadable() == false)
    {
        throw StatesException("MachineSaveFileManager", permission_denied, "Can't read file");
    }

    this->file = shared_ptr<QFile>(new QFile(filePath));
    bool fileOpened = this->file->open(QIODevice::ReadOnly);
    if (fileOpened == false)
    {
        throw StatesException("MachineSaveFileManager", unable_to_open, "Unable to open file");
    }

    this->document = shared_ptr<QDomDocument>(new QDomDocument());
    if (this->document->setContent(this->file.get()) == false)
    {
        throw StatesException("MachineSaveFileManager", wrong_xml, "Unable to open document as a correct XML file");
    }
}

void MachineSaveFileManager::closeLoadFile()
{
    this->file->close();
}

void MachineSaveFileManager::parseMachineName(QDomElement rootNode, shared_ptr<Machine> machine)
{
    QString machineName = rootNode.attribute("Name");
    bool noName = false;
    if (machineName == QString::null)
    {
        machineName = this->file->fileName();
        machineName = machineName.section("/", -1, -1);             // Extract file name from path
        machineName.remove("." + machineName.section(".", -1, -1)); // Remove extension
        noName = true;
    }
    if (machineName == QString::null) // In case we still have no name (file with no file name?)
    {
        machineName = tr("Machine");
        noName = true;
    }
    if (noName == true)
    {
        this->warnings.append(tr("No name was found for the machine."));
        this->warnings.append("    " + tr("Name defaulted to:") + " " + machineName + ".");
    }
    machine->setName(machineName);
}

void MachineSaveFileManager::parseMachineConfiguration(QDomElement element)
{
    QDomNodeList signalNodes = element.childNodes();
    this->configuration = shared_ptr<MachineConfiguration>(new MachineConfiguration());

    for (int i = 0 ; i < signalNodes.count() ; i++)
    {
        QDomElement currentElement = signalNodes.at(i).toElement();

        if (currentElement.tagName() == "Scale")
        {
            bool ok;
            float level = currentElement.attribute("Value").toFloat(&ok);
            if (ok == false)
            {
                continue;
            }

            this->configuration->zoomLevel = level;
        }
        else if (currentElement.tagName() == "ViewCentralPoint")
        {
            bool ok;
            float x = currentElement.attribute("X").toFloat(&ok);
            if (ok == false)
            {
                continue;
            }

            float y = currentElement.attribute("Y").toFloat(&ok);
            if (ok == false)
            {
                continue;
            }

            this->configuration->viewCenter = QPointF(x,y);
        }
    }
}

void MachineSaveFileManager::parseMachineSignals(QDomElement element, shared_ptr<Machine> machine)
{
    QDomNodeList signalNodes = element.childNodes();
    QString currentElementName;

    for (int i = 0 ; i < signalNodes.count() ; i++)
    {
        QDomElement currentElement = signalNodes.at(i).toElement();
        currentElementName = currentElement.attribute("Name");

        if (currentElementName.isEmpty())
        {
            this->warnings.append(tr("Unnamed signal encountered while parsing signal list."));
            this->warnings.append("    " + tr("Token ignored."));
            continue;
        }

        shared_ptr<Signal> signal;
        if (currentElement.tagName() == "Input")
        {
            signal = machine->addSignal(Machine::signal_type::Input, currentElementName);
        }
        else if (currentElement.tagName() == "Output")
        {
            signal = machine->addSignal(Machine::signal_type::Output, currentElementName);
        }
        else if (currentElement.tagName() == "Variable")
        {
            signal = machine->addSignal(Machine::signal_type::LocalVariable, currentElementName);
        }
        else if (currentElement.tagName() == "Constant")
        {
            signal = machine->addSignal(Machine::signal_type::Constant, currentElementName);
        }
        else
        {
            this->warnings.append(tr("Unexpected signal type encountered while parsing signal list:"));
            this->warnings.append("    " + tr("Expected") + " \"Input\", \"Output\", \"Variable\" " + tr("or") + " \"Constant\", " + tr("got") + " \"" + currentElement.tagName() + "\".");
            this->warnings.append("    " + tr("Signal name was:") + " " + currentElementName + ".");
            this->warnings.append("    " + tr("Token ignored."));
            continue;
        }

        // Set size
        uint size = (uint)currentElement.attribute("Size").toInt();
        if (size != 1)
        {
            try
            {
                machine->resizeSignal(currentElementName, size); // Throws StatesException (Signal, Equation and Machine)
                // Equation: ignored, this is not an equation,
                // Machine: ignored, we just created the signal,
                // Only Signal has to be handled
            }
            catch (const StatesException& e)
            {
                if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::resized_to_0) )
                {
                    this->warnings.append(tr("Unable to resize signal") + " \"" + currentElementName + "\".");
                    this->warnings.append("    " + tr("Signal size ignored and defaulted to") + " \"" + QString::number(signal->getSize()) + "\".");
                }
                else
                    throw;
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
                if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
                {
                    this->warnings.append("Error in initial value of signal " + currentElementName + ".");
                    this->warnings.append("    " + tr("Given initial value was") + " \"" + currentElement.attribute("Initial_value") + "\".");
                    this->warnings.append("    " + tr("Initial value ignored and defaulted to") + " \"" + QString::number(signal->getSize()) + "\".");
                }
                else if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::size_mismatch) )
                {
                    this->warnings.append("Error in initial value of signal " + currentElementName + ".");
                    this->warnings.append("    " + tr("The initial value size does not match signal size."));
                    this->warnings.append("    " + tr("Initial value ignored and defaulted to") + " \"" + QString::number(signal->getSize()) + "\".");
                }
                else
                    throw;
            }
        }
    }
}

void MachineSaveFileManager::parseActuatorActions(QDomElement element, shared_ptr<MachineActuatorComponent> component, shared_ptr<Machine> machine)
{
    QDomNodeList childNodes = element.childNodes();

    for (int i = 0 ; i < childNodes.count() ; i++)
    {
        QDomElement currentElement = childNodes.at(i).toElement();

        if (currentElement.tagName() != "Action")
        {
            this->warnings.append(tr("Unexpected token encountered while parsing action list:"));
            this->warnings.append("    " + tr("Expected") + " \"Action\", " + tr("got") + " \"" + currentElement.tagName() + "\".");
            this->warnings.append("    " + tr("Action ignored."));
            continue;
        }

        QString signalName = currentElement.attribute("Name");

        shared_ptr<Signal> signal;
        foreach (shared_ptr<Signal> var, machine->getWrittableSignals())
        {
            if (var->getName() == signalName)
                signal = var;
        }

        if (signal == nullptr)
        {
            this->warnings.append(tr("Reference to undeclared signal encountered while parsing action list:"));
            this->warnings.append("    " + tr("Signal name was") + " \"" + signalName + "\".");
            this->warnings.append("    " + tr("Action ignored."));
            continue;
        }

        ActionOnSignal::action_types actionType;
        QString actionTypeText = currentElement.attribute("Action_Type");

        if (actionTypeText == "Pulse")
        {
            actionType = ActionOnSignal::action_types::pulse;
        }
        else if (actionTypeText == "ActiveOnState")
        {
            actionType = ActionOnSignal::action_types::activeOnState;
        }
        else if (actionTypeText == "Set")
        {
            actionType = ActionOnSignal::action_types::set;
        }
        else if (actionTypeText == "Reset")
        {
            actionType = ActionOnSignal::action_types::reset;
        }
        else if (actionTypeText == "Assign")
        {
            actionType = ActionOnSignal::action_types::assign;
        }
        else
        {
            this->warnings.append(tr("Unexpected action type encountered while parsing action list:"));
            this->warnings.append("    " + tr("Action type was") + " \"" + actionTypeText + "\".");
            this->warnings.append("    " + tr("Action ignored."));
            continue;
        }

        shared_ptr<ActionOnSignal> action = component->addAction(signal);

        try
        {
            action->setActionType(actionType); // Throws StatesException
        }
        catch (const StatesException& e)
        {
            if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_type) )
            {
                this->warnings.append(tr("Error in action type for signal") + " \"" + signalName + "\".");
                this->warnings.append("    " + tr("Default action type used instead."));
            }
            else
                throw;
        }

        QString srangel = currentElement.attribute("RangeL");
        QString sranger = currentElement.attribute("RangeR");
        QString sactval = currentElement.attribute("Action_Value");
        // For compatibility with previous saves
        if (srangel.isNull())
            srangel = currentElement.attribute("Param1");
        if (sranger.isNull())
            sranger = currentElement.attribute("Param2");

        int rangeL;
        int rangeR;

        if (! srangel.isEmpty())
        {
            rangeL = srangel.toInt();
        }
        else
            rangeL = -1;

        if (! sranger.isEmpty())
        {
            rangeR = sranger.toInt();
        }
        else
            rangeR = -1;

        try
        {
            action->setActionRange(rangeL, rangeR);
        }
        catch (const StatesException& e)
        {
            if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_range) )
            {
                this->warnings.append(tr("Error in action range for signal") + " \"" + signalName + "\".");
                this->warnings.append("    " + tr("Range ignored. Default value will be ignored too if present."));
                continue;
            }
            else
                throw;
        }

        if(! sactval.isEmpty())
        {
            LogicValue actionValue;
            try
            {
                actionValue = LogicValue::fromString(sactval); // Throws StatesException
            }
            catch (const StatesException& e)
            {
                if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
                {
                    uint avsize;
                    if ( (rangeL != -1) && (rangeR == -1) )
                        avsize = 1;
                    else if ( (rangeL != -1) && (rangeR != -1) )
                        avsize = rangeL - rangeR + 1;
                    else
                        avsize = 1; // TODO: determine actual size

                    actionValue = LogicValue::getValue0(avsize);

                    this->warnings.append(tr("Error in action value for signal") + " \"" + signalName + "\".");
                    this->warnings.append("    " + tr("Value ignored and set to") + " \"" + actionValue.toString() + "\".");
                    continue;
                }
                else
                    throw;
            }

            if (action->isActionValueEditable() == true)
            {
                try
                {
                    action->setActionValue(actionValue);
                }
                catch (const StatesException& e)
                {
                    if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_value) )
                    {
                        this->warnings.append(tr("Error in action value for signal") + " \"" + signalName + "\".");
                        this->warnings.append("    " + tr("Value ignored and set to") + " \"" + action->getActionValue().toString() + "\".");
                    }
                    else
                        throw;
                }
            }
        }
    }
}

shared_ptr<Signal> MachineSaveFileManager::parseEquation(QDomElement element, shared_ptr<Machine> machine)
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
            int rangeL = -1;
            int rangeR = -1;
            LogicValue constantValue;
            QString srangel;
            QString sranger;

            // TODO: use operand count to increase operand map
            // if we obtain a correct value for attribute.
            // This would preserve empty operands at vector end.
            //int operandCount = currentElement.attribute("OperandCount").toInt();

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

                srangel = currentElement.attribute("RangeL");
                sranger = currentElement.attribute("RangeR");
                // For compatibility with previous saves
                if (srangel.isNull())
                    srangel = currentElement.attribute("Param1");
                if (sranger.isNull())
                    sranger = currentElement.attribute("Param2");

                rangeL = srangel.toInt();
                rangeR = sranger.toInt();
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
                        if ( (rangeL != -1) && (rangeR == -1) )
                            constantsize = 1;
                        else if ( (rangeL != -1) && (rangeR != -1) )
                            constantsize = rangeL - rangeR + 1;
                        else
                            constantsize = 1; // TODO: determine actual size

                        constantValue = LogicValue::getValue0(constantsize);

                        this->warnings.append(tr("Error in constant value while parsing equation:"));
                        this->warnings.append("    " + tr("Value ignored and set to") + " \"" + constantValue.toString() + "\".");
                    }
                    else
                        throw;
                }
            }
            else
            {
                this->warnings.append(tr("Unexpected equation nature encountered while parsing logic equation:"));
                this->warnings.append("    " + tr("Equation nature was:") + " \"" + currentElement.attribute("Nature") + "\".");
                this->warnings.append("    " + tr("Token ignored. Will retry with other tokens if existing."));
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
                    this->warnings.append(tr("Unexpected token encountered while parsing logic equation:"));
                    this->warnings.append("    " + tr("Expected") + " \"Operand\", " + tr("got") + " \"" + currentElement.tagName() + "\".");
                    this->warnings.append("    " + tr("Token ignored. Will retry with other tokens if existing."));
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

            shared_ptr<Equation> newEquation = shared_ptr<Equation>(new Equation(equationType, operands));

            if (equationType == Equation::nature::constant)
            {
                newEquation->setConstantValue(constantValue); // Throws StatesException - constantValue is built for signal size - ignored
            }
            else if (equationType == Equation::nature::extractOp)
            {
                newEquation->setRange(rangeL, rangeR);
            }

            equation = newEquation;
        }
        else
        {
            this->warnings.append(tr("Unexpected token encountered while parsing logic equation:"));
            this->warnings.append("    " + tr("Expected") + " " + tr("signal") + " (LogicVariable) " + tr("or") + " " + tr("equation") + " (LogicEquation), " + tr("got") + "  \"" + currentElement.tagName() + "\".");
            this->warnings.append("    " + tr("Token ignored. Will retry with other tokens if existing."));
            continue;
        }
    }

    return equation;
}

void MachineSaveFileManager::createSaveFile(const QString& filePath) // Throws StatesException
{
    QFileInfo fileInfo(filePath);
    if ( (fileInfo.exists()) && (!fileInfo.isWritable()) ) // Replace existing file
    {
        throw StatesException("MachineSaveFileManager", unable_to_replace, "Unable to replace existing file");
    }
    else if ( !fileInfo.absoluteDir().exists() )
    {
        throw StatesException("MachineSaveFileManager", unkown_directory, "Directory doesn't exist");
    }

    this->file = unique_ptr<QFile>(new QFile(filePath));
    bool fileOpened = file->open(QIODevice::WriteOnly);
    if (fileOpened == false)
    {
        throw StatesException("MachineSaveFileManager", unable_to_open, "Unable to open file");
    }

    this->stream = shared_ptr<QXmlStreamWriter>(new QXmlStreamWriter(this->file.get()));

    this->stream->setAutoFormatting(true);
    this->stream->writeStartDocument();
}

void MachineSaveFileManager::createSaveString()
{
    this->xmlString = QString();
    this->stream = shared_ptr<QXmlStreamWriter>(new QXmlStreamWriter(&this->xmlString));
}

void MachineSaveFileManager::finalizeSaveFile()
{
    this->stream->writeEndDocument();

    this->file->close();
    this->file.reset();
}
