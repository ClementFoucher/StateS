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
#include <QXmlStreamWriter>

// StateS classes
#include "statesexception.h"
#include "machineconfiguration.h"
#include "machine.h"
#include "signal.h"
#include "constant.h"
#include "input.h"
#include "output.h"


MachineSaveFileManager::MachineSaveFileManager(QObject* parent) :
    QObject(parent)
{
    this->warnings = QList<QString>();
}

shared_ptr<MachineConfiguration> MachineSaveFileManager::getConfiguration()
{
    if (this->configuration != nullptr)
    {
        return this->configuration;
    }
    else
    {
        return shared_ptr<MachineConfiguration>(new MachineConfiguration());
    }
}

void MachineSaveFileManager::writeConfiguration(QXmlStreamWriter& stream, shared_ptr<MachineConfiguration> configuration)
{
    stream.writeStartElement("Configuration");

    stream.writeStartElement("VisibleArea"); // Visible area after translation
    stream.writeAttribute("X",      QString::number(configuration->sceneVisibleArea.left() + configuration->sceneTranslation.x()));
    stream.writeAttribute("Y",      QString::number(configuration->sceneVisibleArea.top()  + configuration->sceneTranslation.y()));
    stream.writeAttribute("Width",  QString::number(configuration->sceneVisibleArea.width()));
    stream.writeAttribute("Height", QString::number(configuration->sceneVisibleArea.height()));
    stream.writeEndElement();

    stream.writeEndElement();
}

void MachineSaveFileManager::writeSignals(QXmlStreamWriter& stream, shared_ptr<Machine> machine)
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

void MachineSaveFileManager::parseConfiguration(QDomElement element)
{
    QDomNodeList signalNodes = element.childNodes();
    this->configuration = shared_ptr<MachineConfiguration>(new MachineConfiguration());

    for (int i = 0 ; i < signalNodes.count() ; i++)
    {
        QDomElement currentElement = signalNodes.at(i).toElement();

        if (currentElement.tagName() == "VisibleArea")
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
            float width  = currentElement.attribute("Width").toFloat(&ok);
            if (ok == false)
            {
                continue;
            }
            float height = currentElement.attribute("Height").toFloat(&ok);
            if (ok == false)
            {
                continue;
            }

            QRectF visibleArea(x, y, width, height);

            this->configuration->sceneVisibleArea = visibleArea;
        }
    }
}

void MachineSaveFileManager::parseSignals(QDomElement element, shared_ptr<Machine> machine)
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
