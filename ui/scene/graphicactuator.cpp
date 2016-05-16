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
#include "graphicactuator.h"

// Qt classes
#include <QBrush>
#include <QGraphicsItem>

// StateS classes
#include "machineactuatorcomponent.h"
#include "signal.h"
#include "machine.h"
#include "statesexception.h"


GraphicActuator::GraphicActuator(QObject* parent) :
    GraphicComponent(parent)
{
    this->initialize();
}

GraphicActuator::GraphicActuator(shared_ptr<MachineActuatorComponent> actuator, QObject* parent) :
    GraphicComponent(dynamic_pointer_cast<MachineComponent>(actuator), parent)
{
    this->initialize();
}

GraphicActuator::~GraphicActuator()
{
    delete actionsBox;
}

QGraphicsItemGroup* GraphicActuator::getActionsBox() const
{
    return this->actionsBox;
}

void GraphicActuator::setLogicActuator(shared_ptr<MachineActuatorComponent> actuator) // Throws StatesException
{
    this->setLogicComponent(dynamic_pointer_cast<MachineComponent>(actuator)); // Throws StatesException - propagated
}

shared_ptr<MachineActuatorComponent> GraphicActuator::getLogicActuator() // Throws StatesException
{
    return dynamic_pointer_cast<MachineActuatorComponent>(this->getLogicComponent()); // Throws StatesException - propagated
}

void GraphicActuator::buildActionsBox(const QPen& pen, bool center)
{
    // Clean
    qDeleteAll(actionsBox->childItems());
    actionsBox->childItems().clear();

    // Check base reference
    shared_ptr<MachineActuatorComponent> l_actuator;

    try
    {
        l_actuator = this->getLogicActuator();
    }
    catch (const StatesException& e)
    {
        if ( (e.getSourceClass() == "GraphicComponent") && (e.getEnumValue() == GraphicComponent::GraphicComponentErrorEnum::obsolete_base_object) )
        {
            // Just ignore building action
            return;
        }
        else
            throw;
    }

    // Begin build
    QList<shared_ptr<Signal>> actions = l_actuator->getActions();
    if (actions.count() != 0)
    {
        qreal textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();
        qreal maxTextWidth = 0;

        for (int i = 0 ; i < actions.count() ; i++)
        {
            QGraphicsTextItem* actionText = new QGraphicsTextItem(actions[i]->getText(), actionsBox);

            QString currentActionText;

            Machine::mode currentMode = l_actuator->getOwningMachine()->getCurrentMode();

            if (currentMode == Machine::mode::simulateMode)
                currentActionText = actions[i]->getText(true);
            else
                currentActionText = actions[i]->getText(false);

            if (actions[i]->getSize() > 1)
            {
                int param1 = l_actuator->getActionParam1(actions[i]);
                int param2 = l_actuator->getActionParam2(actions[i]);

                if (param1 != -1)
                {
                    currentActionText += "[";
                    currentActionText += QString::number(param1);

                    if (param2 != -1)
                    {
                        currentActionText += "..";
                        currentActionText += QString::number(param2);
                    }

                    currentActionText += "]";
                }
            }

            if ((actions[i]->getSize() > 1) &&  (   (l_actuator->getActionParam1(actions[i]) == -1) ||
                                                    ( (l_actuator->getActionParam1(actions[i]) != -1) && (l_actuator->getActionParam2(actions[i]) != -1) )
                                                    ) )
            {
                // Range vector
                MachineActuatorComponent::action_types type = l_actuator->getActionType(actions[i]);
                if (type == MachineActuatorComponent::action_types::set)
                {
                    currentActionText += " ← " + LogicValue::getValue1(actions[i]->getSize()).toString(); // + "<sub>b</sub>";
                }
                else if (type == MachineActuatorComponent::action_types::reset)
                {
                    currentActionText += " ← " + LogicValue::getValue0(actions[i]->getSize()).toString(); // + "<sub>b</sub>";
                }
                else if (type == MachineActuatorComponent::action_types::assign)
                {
                    currentActionText += " ← " + l_actuator->getActionValue(actions[i]).toString(); // + "<sub>b</sub>";
                }
                else if ( (type == MachineActuatorComponent::action_types::activeOnState) ||
                          (type == MachineActuatorComponent::action_types::pulse) )
                {
                    currentActionText += " ↷ " + l_actuator->getActionValue(actions[i]).toString(); // + "<sub>b</sub>";
                }
            }
            else
            {
                // Single bit
                MachineActuatorComponent::action_types type = l_actuator->getActionType(actions[i]);
                if (type == MachineActuatorComponent::action_types::set)
                {
                    currentActionText += " ← 1"; // + "<sub>b</sub>";
                }
                else if (type == MachineActuatorComponent::action_types::reset)
                {
                    currentActionText += " ← 0"; // + "<sub>b</sub>";
                }
            }

            actionText->setHtml(currentActionText);


            if (maxTextWidth < actionText->boundingRect().width())
                maxTextWidth = actionText->boundingRect().width();

            if (center)
                actionText->setPos(0, -( ( (textHeight*actions.count()) / 2) ) + i*textHeight);
            else
                actionText->setPos(QPointF(0, i*actionText->boundingRect().height()));

            actionText->setZValue(1);
        }

        QPainterPath actionBorderPath;
        if (center)
        {
            actionBorderPath.lineTo(0,                 ((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(0 + maxTextWidth,  ((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(0 + maxTextWidth, -((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(0,                -((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(0,                0);
            actionBorderPath.lineTo(-20 ,             0);
        }
        else
        {
            actionBorderPath.lineTo(0,            ((qreal)actions.count())*textHeight);
            actionBorderPath.lineTo(maxTextWidth, ((qreal)actions.count())*textHeight);
            actionBorderPath.lineTo(maxTextWidth, 0);
            actionBorderPath.lineTo(0,            0);
        }

        QGraphicsPathItem* stateActionsOutline = new QGraphicsPathItem(actionBorderPath, actionsBox);
        stateActionsOutline->setPen(pen);
        stateActionsOutline->setBrush(QBrush(Qt::white, Qt::Dense3Pattern));
        stateActionsOutline->setZValue(0);
        stateActionsOutline->setPos(0, 0);
    }
}

void GraphicActuator::initialize()
{
    this->actionsBox = new QGraphicsItemGroup();
}
