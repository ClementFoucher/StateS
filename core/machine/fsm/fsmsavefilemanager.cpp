/*
 * Copyright © 2014-2017 Clément Foucher
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
#include <QXmlStreamWriter>
#include <QDomElement>

// StateS classes
#include "fsm.h"
#include "fsmstate.h"
#include "fsmgraphicstate.h"
#include "fsmtransition.h"
#include "statesexception.h"
#include "fsmgraphictransition.h"
#include "machineconfiguration.h"


FsmSaveFileManager::FsmSaveFileManager(QObject* parent) :
    MachineSaveFileManager(parent)
{

}

QString FsmSaveFileManager::getMachineXml(shared_ptr<Machine> machine)
{
    this->createSaveString();
    this->writeFsmToStream(dynamic_pointer_cast<Fsm>(machine), nullptr);
    return this->xmlString;
}

shared_ptr<Machine> FsmSaveFileManager::loadMachineFromXml(const QString& machineXml)
{
    this->initializeDocumentFromString(machineXml);
    return this->loadFsmFromDocument();
}

void FsmSaveFileManager::writeMachineToFile(shared_ptr<Machine> machine, shared_ptr<MachineConfiguration> configuration, const QString& filePath)
{
    this->createSaveFile(filePath);
    this->writeFsmToStream(dynamic_pointer_cast<Fsm>(machine), configuration);
    this->finalizeSaveFile();
}

shared_ptr<Machine> FsmSaveFileManager::loadMachineFromFile(const QString& filePath)
{
    this->initializeDocumentFromFile(filePath);

    shared_ptr<Fsm> machine = this->loadFsmFromDocument();

    this->closeLoadFile();

    return machine;
}

void FsmSaveFileManager::writeFsmStates(shared_ptr<Fsm> machine, shared_ptr<MachineConfiguration> configuration)
{
    this->stream->writeStartElement("States");

    foreach (shared_ptr<FsmState> state, machine->getStates())
    {
        this->stream->writeStartElement("State");

        // Name
        this->stream->writeAttribute("Name", state->getName());

        // Initial
        if (state->isInitial())
            this->stream->writeAttribute("IsInitial", "true");

        if (configuration != nullptr)
        {
            // Position => offseted so that scene top-left corner is in (0,0)
            this->stream->writeAttribute("X", QString::number(state->getGraphicRepresentation()->scenePos().x() + configuration->sceneTranslation.x()));
            this->stream->writeAttribute("Y", QString::number(state->getGraphicRepresentation()->scenePos().y() + configuration->sceneTranslation.y()));
        }
        else
        {
            // Position
            this->stream->writeAttribute("X", QString::number(state->getGraphicRepresentation()->scenePos().x()));
            this->stream->writeAttribute("Y", QString::number(state->getGraphicRepresentation()->scenePos().y()));
        }

        // Actions
        this->writeActuatorActions(state);

        this->stream->writeEndElement();
    }

    this->stream->writeEndElement();
}

void FsmSaveFileManager::writeFsmTransitions(shared_ptr<Fsm> machine)
{
    this->stream->writeStartElement("Transitions");

    foreach (shared_ptr<FsmTransition> transition, machine->getTransitions())
    {
        this->stream->writeStartElement("Transition");

        this->stream->writeAttribute("Source", transition->getSource()->getName());
        this->stream->writeAttribute("Target", transition->getTarget()->getName());

        int sliderPosition = transition->getGraphicRepresentation()->getConditionLineSliderPosition()*100;
        this->stream->writeAttribute("SliderPos", QString::number(sliderPosition));

        // Deal with equations
        if (transition->getCondition() != nullptr)
        {
            this->stream->writeStartElement("Condition");
            this->writeLogicEquation(transition->getCondition());
            this->stream->writeEndElement(); // Condition
        }

        // Actions
        this->writeActuatorActions(transition);

        this->stream->writeEndElement();
    }

    this->stream->writeEndElement();
}

shared_ptr<Fsm> FsmSaveFileManager::loadFsmFromDocument()
{
    QDomElement rootNode = this->document->documentElement();
    if (rootNode.tagName() != "FSM")
    {
        throw StatesException("FsmSaveFileManager", wrong_xml, "Unexpected token found while parsing XML file: expected \"FSM\", got " + rootNode.tagName());
    }

    shared_ptr<Fsm> machine(new Fsm());

    this->parseMachineName(rootNode, machine);

    QDomNodeList fsmNodes = rootNode.childNodes();

    // First parse configuration
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "Configuration")
        {
            this->parseMachineConfiguration(currentElement);
            break;
        }
    }

    // Parse signals
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "Signals")
        {
            this->parseMachineSignals(currentElement, machine);
            break;
        }
    }

    // Then parse states
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "States")
        {
            this->parseFsmStates(currentElement, machine);
            break;
        }
    }

    // Finally parse transitions
    for (int i = 0 ; i < fsmNodes.count() ; i++)
    {
        QDomElement currentElement = fsmNodes.at(i).toElement();

        if (currentElement.tagName() == "Transitions")
        {
            this->parseFsmTransitions(currentElement, machine);
            break;
        }
    }

    return machine;
}

void FsmSaveFileManager::writeFsmToStream(shared_ptr<Fsm> machine, shared_ptr<MachineConfiguration> configuration)
{
    this->stream->writeStartElement("FSM");
    this->stream->writeAttribute("Name", machine->getName());

    this->writeMachineCommonElements(machine, configuration);
    this->writeFsmStates(machine, configuration);
    this->writeFsmTransitions(machine);

    this->stream->writeEndElement(); // End FSM element
}

void FsmSaveFileManager::parseFsmStates(QDomElement element, shared_ptr<Fsm> machine)
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
                    parseActuatorActions(currentElement, state, machine);
                }
                else
                {
                    this->warnings.append(tr("Unexpected token encountered while parsing state list:"));
                    this->warnings.append("    " + tr("Expected") + " \"Actions\"," + tr("got") + " \"" + currentElement.tagName() + "\".");
                    this->warnings.append("    " + tr("Token ignored."));
                    continue;
                }
            }
        }
        else
        {
            this->warnings.append(tr("Unexpected token encountered while parsing state list:"));
            this->warnings.append("    " + tr("Expected") + " \"State\", " + tr("got") + " \"" + currentElement.tagName() + "\".");
            this->warnings.append("    " + tr("Token ignored."));
            continue;
        }
    }
}

void FsmSaveFileManager::parseFsmTransitions(QDomElement element, shared_ptr<Fsm> machine)
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

            QString sliderPosString = currentElement.attribute("SliderPos");
            qreal sliderPos;
            if (!sliderPosString.isEmpty())
            {
                sliderPos = sliderPosString.toFloat()/100;
            }
            else
            {
                sliderPos = 0.5;
            }
            transition->sliderPos = sliderPos;

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
                    parseActuatorActions(currentElement, transition, machine);
                }
                else
                {
                    this->warnings.append(tr("Unexpected token encountered while parsing transition list:"));
                    this->warnings.append("    " + tr("Expected") + " \"Condition\" " + tr("or") + " \"Actions\", " + tr("got") + " \"" + currentElement.tagName() + "\".");
                    this->warnings.append("    " + tr("Token ignored."));
                    continue;
                }
            }
        }
        else
        {
            this->warnings.append(tr("Unexpected token encountered while parsing transition list:"));
            this->warnings.append("    " + tr("Expected") + " \"Transition\", " + tr("got") + " \"" + currentElement.tagName() + "\".");
            this->warnings.append("    " + tr("Token ignored."));
            continue;
        }
    }
}
