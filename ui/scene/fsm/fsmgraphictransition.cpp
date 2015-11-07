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
#include "fsmgraphictransition.h"

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsView>

// Debug
#include <QDebug>

// States classes
#include "fsmgraphicstate.h"
#include "fsmstate.h"
#include "machine.h"
#include "fsmtransition.h"
#include "fsmgraphictransitionneighborhood.h"
#include "signal.h"
#include "contextmenu.h"
#include "fsm.h"


qreal FsmGraphicTransition::arrowEndSize = 10;
qreal FsmGraphicTransition::middleBarLength = 20;
QPen FsmGraphicTransition::selectionPen = QPen(Qt::DashLine);
QPen FsmGraphicTransition::standardPen = QPen(Qt::SolidPattern, 3);
QPen FsmGraphicTransition::editPen = QPen(QBrush(Qt::red, Qt::SolidPattern), 3);
QPen FsmGraphicTransition::activePen = QPen(QBrush(Qt::darkGreen, Qt::SolidPattern), 3);
QPen FsmGraphicTransition::inactivePen = QPen(QBrush(Qt::red, Qt::SolidPattern), 3);

QPixmap FsmGraphicTransition::getPixmap(uint size)
{
    QPixmap pixmap(QSize(size, size));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    painter.setPen(FsmGraphicTransition::standardPen);
    painter.drawLine(0, 0, size, size);
    painter.drawLine(0, 0, size/3, 0);
    painter.drawLine(0, 0, 0, size/3);

    return pixmap;
}

FsmGraphicTransition::FsmGraphicTransition()
{
    currentPen = &standardPen;

    conditionText = new QGraphicsTextItem();
    actionsBox    = new QGraphicsItemGroup();

    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsFocusable);
}

FsmGraphicTransition::FsmGraphicTransition(FsmGraphicState* source, const QPointF& dynamicMousePosition) :
    FsmGraphicTransition()
{
    currentMode = mode::initMode;
    this->setSourceState(source);
    this->setDynamicTargetMode(dynamicMousePosition);
    // We are now in dynamic target mode

    finishInitialize();
}

FsmGraphicTransition::FsmGraphicTransition(shared_ptr<FsmTransition> logicTransition) :
    FsmGraphicTransition()
{
    logicTransition->setGraphicRepresentation(this);
    connect(logicTransition.get(), &MachineComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicTransition::updateText);
    connect(logicTransition.get(), &MachineComponent::componentDynamicStateChangedEvent,        this, &FsmGraphicTransition::updateText);

    shared_ptr<Fsm> machine = logicTransition->getOwningFsm();
    connect(machine.get(), &Fsm::changedModeEvent, this, &FsmGraphicTransition::machineModeChangedEventHandler);

    currentMode = mode::initMode;
    this->setSourceState(logicTransition->getSource()->getGraphicRepresentation());
    this->setTargetState(logicTransition->getTarget()->getGraphicRepresentation());
    currentMode = mode::standardMode;

    this->logicTransition = logicTransition;

    checkNeighboors();
    finishInitialize();
}

void FsmGraphicTransition::finishInitialize()
{
    rebuildArrowEnd();
    updateText();
    updateDisplay();
}

FsmGraphicTransition::~FsmGraphicTransition()
{
    quitNeighboorhood();

    delete conditionText;
    delete actionsBox;

    shared_ptr<FsmTransition> l_logicTransition = this->logicTransition.lock();

    if (l_logicTransition != nullptr)
    {
        disconnect(l_logicTransition.get(), &MachineComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicTransition::updateText);
        disconnect(l_logicTransition.get(), &MachineComponent::componentDynamicStateChangedEvent,        this, &FsmGraphicTransition::updateText);


        shared_ptr<Fsm> machine = l_logicTransition->getOwningFsm();
        if (machine != nullptr)
            disconnect(machine.get(), &Fsm::changedModeEvent, this, &FsmGraphicTransition::machineModeChangedEventHandler);

        l_logicTransition->clearGraphicRepresentation();
    }
}

void FsmGraphicTransition::setLogicTransition(shared_ptr<FsmTransition> transition)
{
    // This function can be called only once, we never reaffect a graphic transition
    if (this->logicTransition.expired())
    {
        connect(transition.get(), &MachineActuatorComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicTransition::updateText);
        connect(transition.get(), &FsmTransition::componentDynamicStateChangedEvent,                    this, &FsmGraphicTransition::updateText);

        this->logicTransition = transition;

        this->updateText();
    }
    else
        qDebug() << "(FSM Graphic Transition:) ERROR! Trying to reaffect logic transition while already linked! Ignored call.";
}

shared_ptr<FsmTransition> FsmGraphicTransition::getLogicTransition() const
{
    return this->logicTransition.lock();
}

bool FsmGraphicTransition::setSourceState(FsmGraphicState* newSource)
{
    autoTransitionNeedsRedraw = true;

    if (currentMode == mode::initMode)
    {
        this->source = newSource;

        // Connect to new state
        connect(this->source, &FsmGraphicState::stateMovingEvent, this, &FsmGraphicTransition::updateDisplay);

        return true;
    }
    else if (currentMode == mode::standardMode)
    {
        if (newSource == nullptr) // We can't change source for nullptr
            return false;
        else if (newSource == this->source)
            return true;
        else
        {
            // Disconnect existing signal
            // If we were a single-state transition, do not disconnect as we still have target connected!
            if (this->source != this->target)
                disconnect(this->source, &FsmGraphicState::stateMovingEvent, this, &FsmGraphicTransition::updateDisplay);

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newSource)
                quitNeighboorhood();

            this->source = newSource;

            // Connect to new state
            connect(this->source, &FsmGraphicState::stateMovingEvent, this, &FsmGraphicTransition::updateDisplay);

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newSource)
            {
                checkNeighboors();
                updateDisplay();
            }

            return true;
        }
    }
    else if (currentMode == mode::dynamicSourceMode)
    {
        if (newSource == this->dynamicState)
            return true;
        else
        {
            quitNeighboorhood();

            dynamicState = newSource;

            checkNeighboors();
            updateDisplay();

            return true;
        }
    }
    else
        return false;

}

bool FsmGraphicTransition::setTargetState(FsmGraphicState* newTarget)
{
    autoTransitionNeedsRedraw = true;

    if (currentMode == mode::initMode)
    {
        this->target = newTarget;

        // Connect to new state
        connect(this->target, &FsmGraphicState::stateMovingEvent, this, &FsmGraphicTransition::updateDisplay);

        return true;
    }
    else if (currentMode == mode::standardMode)
    {
        if (newTarget == nullptr) // We can't change target for nullptr
            return false;
        else if (newTarget == this->target)
            return true;
        else
        {
            // Disconnect existing signal, if existing (state can be created without target graphically)
            if (target != nullptr)
            {
                // If we were a single-state transition, do not disconnect as we still have source connected!
                if (this->source != this->target)
                    disconnect(this->target, &FsmGraphicState::stateMovingEvent, this, &FsmGraphicTransition::updateDisplay);
            }

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newTarget)
                quitNeighboorhood();


            this->target = newTarget;

            // Connect to new state
            connect(this->target, &FsmGraphicState::stateMovingEvent, this, &FsmGraphicTransition::updateDisplay);

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newTarget)
            {
                checkNeighboors();
                updateDisplay();
            }

            return true;
        }
    }
    else if (currentMode == mode::dynamicTargetMode)
    {
        if (newTarget == this->dynamicState)
            return true;
        else
        {
            quitNeighboorhood();
            dynamicState = newTarget;
            checkNeighboors();
            updateDisplay();

            return true;
        }
    }
    else
        return false;
}

void FsmGraphicTransition::setMousePosition(const QPointF& mousePos)
{
    this->dynamicState= nullptr;
    this->mousePosition = mousePos;

    quitNeighboorhood();
    updateDisplay();
}

FsmGraphicState* FsmGraphicTransition::getSource() const
{
    return this->source;
}

FsmGraphicState* FsmGraphicTransition::getTarget() const
{
    return this->target;
}

void FsmGraphicTransition::rebuildArrowEnd()
{
    delete arrowEnd;
    arrowEnd = new QGraphicsItemGroup(this);

    //QPainterPath arrowPath;
    QGraphicsLineItem* arrowEnd1 = new QGraphicsLineItem(arrowEnd);
    QGraphicsLineItem* arrowEnd2 = new QGraphicsLineItem(arrowEnd);

    arrowEnd1->setPen(*currentPen);
    arrowEnd2->setPen(*currentPen);

    // Original arrow point north-west
    arrowEnd1->setLine(0, 0, arrowEndSize, 0);
    arrowEnd2->setLine(0, 0, 0, arrowEndSize);
}

void FsmGraphicTransition::treatSelectionBox()
{
    delete selectionBox;
    selectionBox = nullptr;

    if (this->isSelected())
    {
        QGraphicsLineItem* arrowBodyStraightLine = dynamic_cast<QGraphicsLineItem*>(arrowBody);
        QGraphicsPathItem* arrowBodyPath = dynamic_cast<QGraphicsPathItem*>(arrowBody);

        if (arrowBodyStraightLine != nullptr)
        {
            QLineF straightLine = arrowBodyStraightLine->line();

            this->selectionBox = new QGraphicsRectItem(QRectF(0, -middleBarLength/2, straightLine.length(), middleBarLength), this);
            this->selectionBox->setRotation(-straightLine.angle());
            this->selectionBox->setPen(selectionPen);
        }
        else if (arrowBodyPath != nullptr)
        {
            // Special case: the path can be actually a straight line
            if ( (neighbors != nullptr) && (neighbors->count() % 2 == 1) && (neighbors->whatIsMyRank(this) == 0) )
            {
                QLineF straightLine = QLineF(QPointF(0, 0), arrowBodyPath->boundingRect().bottomRight());

                this->selectionBox = new QGraphicsRectItem(QRectF(0, -middleBarLength/2, straightLine.length(), middleBarLength), this);
                this->selectionBox->setRotation(-sceneAngle);
            }
            else
            {
                this->selectionBox = new QGraphicsRectItem(arrowBody->boundingRect() , this);
                this->selectionBox->setRotation(-sceneAngle);
            }

            this->selectionBox->setPen(selectionPen);
        }
        else
            qDebug() << "(Fsm graphic transition:) Error white treating selection box: unknown shape!";
    }
}

void FsmGraphicTransition::updateText()
{
    shared_ptr<FsmTransition> l_logicTransition = this->logicTransition.lock();

    //
    // Condition

    // Should also make background semi-transparent... (we could avoid color?)
    if (l_logicTransition != nullptr)
    {
        Machine::mode currentMode = l_logicTransition->getOwningFsm()->getCurrentMode();

        if ( (scene() != nullptr) && (currentMode == Machine::mode::simulateMode) )
        {
            if (l_logicTransition->getCondition() == nullptr)
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");
            else
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + l_logicTransition->getCondition()->getText(true) + "</div>");
            if (conditionLine != nullptr)
            {
                if (l_logicTransition->getCondition()->isTrue())
                    conditionLine->setPen(activePen);
                else
                    conditionLine->setPen(inactivePen);
            }
        }
        else
        {
            if (l_logicTransition->getCondition() == nullptr)
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");
            else
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + l_logicTransition->getCondition()->getText() + "</div>");
            if (conditionLine != nullptr)
                conditionLine->setPen(standardPen);
        }
    }
    else
        conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");

    //
    // Actions

    if (l_logicTransition != nullptr)
    {
        qDeleteAll(actionsBox->childItems());
        actionsBox->childItems().clear();

        QList<shared_ptr<Signal>> actions = l_logicTransition->getActions();

        qreal maxTextWidth = 0;

        Machine::mode currentMode = l_logicTransition->getOwningFsm()->getCurrentMode();

        for (int i = 0 ; i < actions.count() ; i++)
        {
            QGraphicsTextItem* actionText = new QGraphicsTextItem(actions[i]->getName(), actionsBox);

            QString currentActionText;

            if ( (scene() != nullptr) && (currentMode == Machine::mode::simulateMode) )
                currentActionText = actions[i]->getText(true);
            else
                currentActionText = actions[i]->getText(false);


            if (l_logicTransition->getActionType(actions[i]) == MachineActuatorComponent::action_types::set)
                currentActionText += " = 1";
            else if (l_logicTransition->getActionType(actions[i]) == MachineActuatorComponent::action_types::reset)
                currentActionText += " = 0";

            actionText->setHtml(currentActionText);

            if (maxTextWidth < actionText->boundingRect().width())
                maxTextWidth = actionText->boundingRect().width();
            actionText->setPos(QPointF(0, i*actionText->boundingRect().height()));
            actionText->setZValue(1);
        }

        qreal textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();

        QPainterPath actionBorderPath;
        actionBorderPath.lineTo(0,            ((qreal)actions.count())*textHeight);
        actionBorderPath.lineTo(maxTextWidth, ((qreal)actions.count())*textHeight);
        actionBorderPath.lineTo(maxTextWidth, 0);
        actionBorderPath.lineTo(0,            0);

        QGraphicsPathItem* stateActionsOutline = new QGraphicsPathItem(actionBorderPath, actionsBox);
        stateActionsOutline->setPen(*currentPen);
        stateActionsOutline->setBrush(QBrush(Qt::white, Qt::Dense3Pattern));
        stateActionsOutline->setZValue(0);
    }
}

void FsmGraphicTransition::updateDisplay()
{
    //
    // First deal with mates as a neighboorhood change calls this function
    if (neighbors != nullptr)
    {
        if (neighbors->count() == 1)
        {
            delete neighbors;
            neighbors = nullptr;
        }
    }

    //
    // Determine source and target points depending on mode.
    // For ends that are connected to a state, center is used as a first approximation
    QPointF currentSourcePoint;
    QPointF currentTargetPoint;
    FsmGraphicState* currentSourceState = nullptr;
    FsmGraphicState* currentTargetState = nullptr;

    if (currentMode == mode::standardMode)
    {
        currentSourcePoint = source->scenePos();
        currentTargetPoint = target->scenePos();

        currentSourceState = source;
        currentTargetState = target;
    }
    else if (currentMode == mode::dynamicSourceMode)
    {
        if (dynamicState != nullptr)
        {
            currentSourcePoint = dynamicState->scenePos();
            currentSourceState = dynamicState;
        }
        else
        {
            currentSourcePoint = mousePosition;
            currentSourceState = nullptr;
        }

        currentTargetPoint = target->scenePos();
        currentTargetState = target;
    }
    else if (currentMode == mode::dynamicTargetMode)
    {
        currentSourcePoint = source->scenePos();
        currentSourceState = source;

        if (dynamicState != nullptr)
        {
            currentTargetPoint = dynamicState->scenePos();
            currentTargetState = dynamicState;
        }
        else
        {
            currentTargetPoint = mousePosition;
            currentTargetState = nullptr;
        }
    }

    //
    // Redraw arrow body
    QPointF curveMiddle;

    if ( (neighbors == nullptr) && (currentSourceState != currentTargetState) )
    {
        delete this->conditionLine;
        delete this->arrowBody;

        this->conditionLine = nullptr;
        this->arrowBody     = nullptr;

        // If no mates, draw a straight line

        QLineF straightLine(QPointF(0, 0), currentTargetPoint - currentSourcePoint);

        // Set line appropriate size (reduce current size by correct number of state radius to point on states border)
        if ( ((currentMode == mode::dynamicSourceMode) || (currentMode == mode::dynamicTargetMode)) && (dynamicState == nullptr) )
        {
            // Reduce line length to match space between state center and mouse position
            straightLine.setLength(straightLine.length() - FsmGraphicState::getRadius());
        }
        else
        {
            // Reduce line length to match space between states
            straightLine.setLength(straightLine.length() - 2*FsmGraphicState::getRadius());
        }

        // Create line graphic representation
        QGraphicsLineItem* line = new QGraphicsLineItem(this);
        //   line->setFlag(QGraphicsItem::ItemIsSelectable);
        line->setPen(*currentPen);
        line->setLine(straightLine);
        arrowBody = line;

        // Display condition
        QLineF conditionLineF = straightLine.normalVector();
        conditionLineF.setLength(middleBarLength);
        conditionLine = new QGraphicsLineItem(conditionLineF, line);
        conditionLine->setPen(*currentPen);
        curveMiddle = QPointF(straightLine.p2()/2);
        conditionLine->setPos(curveMiddle - conditionLineF.p2()/2);

        //
        // Update positions with actual ones used for construction

        // If source is a state, line should be translated to begin on state border
        if ( !((currentMode == mode::dynamicSourceMode) && (dynamicState == nullptr)) )
        {
            // Translation vector is based on straight line (same vector) and normalized to required translation length
            QLineF translationVector(straightLine);
            translationVector.setLength(FsmGraphicState::getRadius());

            currentSourcePoint += translationVector.p2();
        }

        // Current target position is calculated based on source point translated by line
        currentTargetPoint = currentSourcePoint + straightLine.p2();

        // Compute arrow end rotation based on line angle
        arrowEnd->setRotation(135-straightLine.angle());
        // Set arrow position to be at target side
        arrowEnd->setPos(currentTargetPoint - currentSourcePoint); // Positions are intended in scene coordinates. Relocate wrt. this item coordinates: (0, 0) is at source point position

        this->setPos(currentSourcePoint);
    }
    else if (currentSourceState == currentTargetState)
    {
        // Auto-transition

        if (autoTransitionNeedsRedraw)
        {
            delete this->conditionLine;
            delete this->arrowBody;

            this->conditionLine = nullptr;
            this->arrowBody     = nullptr;

            uint rank;
            if (neighbors != nullptr)
                rank = neighbors->whatIsMyRank(this);
            else
                rank = 0;

            QLineF stateCenterToArcStartVector(0, 0, 1, -1);
            stateCenterToArcStartVector.setLength(FsmGraphicState::getRadius());

            QLineF stateCenterToArcEndVector(0, 0, -1, -1);
            stateCenterToArcEndVector.setLength(FsmGraphicState::getRadius());

            qreal unitaryArcWidth  = 2*FsmGraphicState::getRadius();
            qreal unitaryArcHeight = 2*FsmGraphicState::getRadius();

            QLineF stateCenterToUnitaryArcCenterVector(0, 0, 0, 2*stateCenterToArcStartVector.p2().y());

            qreal arcWidth  = unitaryArcWidth  * pow (1.2, rank);
            qreal arcHeight = unitaryArcHeight * pow (1.4, rank);

            QLineF stateCenterToArcCenterVector(0, 0, 0, stateCenterToUnitaryArcCenterVector.p2().y() - (arcHeight-unitaryArcHeight)/2);

            QRectF arcRect(-arcWidth/2, stateCenterToArcCenterVector.p2().y()-arcHeight/2, arcWidth, arcHeight);

            QPainterPath arc;
            arc.moveTo(stateCenterToArcStartVector.p2());


            qreal arcStartAngle = -45;
            qreal arcTotalAngle = 360 - 2* (90 + arcStartAngle);

            arc.arcTo(arcRect, arcStartAngle, arcTotalAngle);

            arc.lineTo(stateCenterToArcEndVector.p2());

            QGraphicsPathItem* arcItem = new QGraphicsPathItem(arc, this);
            arcItem->setPen(*currentPen);
            arrowBody = arcItem;

            QLineF conditionLineF(0, 0, 0, 1);
            conditionLineF.setLength(middleBarLength);
            conditionLine = new QGraphicsLineItem(conditionLineF, arcItem);
            conditionLine->setPen(*currentPen);

            arcMiddle = QPointF(0, stateCenterToArcCenterVector.p2().y()-arcHeight/2);
            conditionLine->setPos(arcMiddle - conditionLineF.p2()/2);

            arrowEnd->setPos(stateCenterToArcEndVector.p2());
            arrowEnd->setRotation(180);

            autoTransitionNeedsRedraw = false;
        }

        curveMiddle = arcMiddle;

        this->setPos(currentSourcePoint);
    }
    else
    {
        delete this->conditionLine;
        delete this->arrowBody;

        this->conditionLine = nullptr;
        this->arrowBody     = nullptr;

        // Take mates in consideration to draw a curve
        // All mates will have a curved body, which curve intensity
        // depends on their rank in mates

        // Get actual source and target in mates's coordinate system
        if ( ((currentMode == mode::standardMode)      && (neighbors->getSource() != this->source)) ||
             ((currentMode == mode::dynamicTargetMode) && (neighbors->getSource() != this->source)) ||
             ((currentMode == mode::dynamicSourceMode) && (neighbors->getSource() != this->dynamicState))
           )
        {
            QPointF temp = currentTargetPoint;
            currentTargetPoint = currentSourcePoint;
            currentSourcePoint = temp;
        }

        // Build arrow body

        QPointF deltaCurveOrigin;
        QPointF curveTarget;
        qreal endAngle1;
        qreal endAngle2;
        arrowBody = neighbors->buildMyBody(currentPen, this, deltaCurveOrigin, curveMiddle, curveTarget, endAngle1, endAngle2, &conditionLine);
        //arrowBody = path;

        sceneAngle = QLineF(QPointF(0,0), curveTarget).angle();

        // Place arrow end on correct side

        if (((currentMode == mode::standardMode)      && (neighbors->getSource() == this->source)) ||
                ((currentMode == mode::dynamicTargetMode) && (neighbors->getSource() == this->source)) ||
                ((currentMode == mode::dynamicSourceMode) && (neighbors->getSource() == this->dynamicState)))
        {
            arrowEnd->setPos(curveTarget);
            arrowEnd->setRotation(135-endAngle1);
        }
        else
        {
            arrowEnd->setPos(0, 0);
            arrowEnd->setRotation(135-endAngle2);
        }

        this->setPos(currentSourcePoint+deltaCurveOrigin);

        // Must be done after scene mapping

    }

    conditionText->setPos(mapToScene(curveMiddle) + QPointF(0, 5));
    actionsBox->setPos(mapToScene(curveMiddle + conditionText->boundingRect().bottomLeft() + QPointF(0, 5)));

    treatSelectionBox();
}

bool FsmGraphicTransition::setDynamicSourceMode(const QPointF& mousePosition)
{
    if (currentMode != mode::standardMode)
        return false;
    else
    {
        this->currentMode = mode::dynamicSourceMode;
        this->currentPen = &editPen;
        rebuildArrowEnd();

        setMousePosition(mousePosition);

        return true;
    }
}

bool FsmGraphicTransition::setDynamicTargetMode(const QPointF& mousePosition)
{
    if ( (currentMode != mode::standardMode) && (currentMode != mode::initMode) )
        return false;
    else
    {
        this->currentMode = mode::dynamicTargetMode;
        this->currentPen = &editPen;
        rebuildArrowEnd();

        setMousePosition(mousePosition);

        return true;
    }
}

bool FsmGraphicTransition::endDynamicMode(bool keepChanges)
{
    shared_ptr<FsmTransition> l_logicTransition = this->logicTransition.lock();

    this->autoTransitionNeedsRedraw = true;

    if (keepChanges)
    {
        if (dynamicState == nullptr)
        {
            return false;
        }
        else if (currentMode == mode::dynamicSourceMode)
        {
            currentMode = mode::standardMode;
            setSourceState(dynamicState);
            if (l_logicTransition != nullptr)
            {
                shared_ptr<FsmState> oldOwner = l_logicTransition->getSource();
                shared_ptr<FsmState> newOwner = dynamicState->getLogicState();

                l_logicTransition->setSource(newOwner);
                oldOwner->removeOutgoingTransition(l_logicTransition);
                newOwner->addOutgoingTransition(l_logicTransition);
            }
        }
        else if (currentMode == mode::dynamicTargetMode)
        {
            currentMode = mode::standardMode;
            setTargetState(dynamicState);
            if (l_logicTransition != nullptr)
            {
                shared_ptr<FsmState> oldTarget = l_logicTransition->getTarget();
                shared_ptr<FsmState> newTarget = dynamicState->getLogicState();

                l_logicTransition->setTarget(newTarget);
                oldTarget->removeIncomingTransition(l_logicTransition);
                newTarget->addIncomingTransition(l_logicTransition);
            }
        }
    }

    dynamicState = nullptr;
    if (!keepChanges)
    {
        currentMode = mode::standardMode;
        quitNeighboorhood();
        checkNeighboors();
    }

    this->currentPen = &standardPen;

    rebuildArrowEnd();
    updateDisplay();

    return true;
}

void FsmGraphicTransition::checkNeighboors()
{
    // Algo is as follows:
    // Check all outcoming transitions from both ends' states (except me)
    // If none => all right, no precautions needed
    // If 1 => initiate a mates secrete society
    // If more than one => join mates secrete society

    FsmGraphicState* actualSourceState;
    FsmGraphicState* actualTargetState;

    if (currentMode == mode::dynamicSourceMode)
    {
        actualSourceState = dynamicState;
        actualTargetState = target;
    }
    else if (currentMode == mode::dynamicTargetMode)
    {
        actualSourceState = source;
        actualTargetState = dynamicState;
    }
    else
    {
        actualSourceState = source;
        actualTargetState = target;
    }

    shared_ptr<FsmTransition> newFriend;

    foreach (shared_ptr<FsmTransition> neighboor, actualSourceState->getLogicState()->getOutgoingTransitions())
    {
        if ( (neighboor->getGraphicRepresentation() != nullptr) && (neighboor->getGraphicRepresentation() != this) ) // Needed, as in dynamic mode I still belong to states!
        {
            if (neighboor->getTarget() == actualTargetState->getLogicState())
            {
                newFriend = neighboor;
                break;
            }
        }
    }

    if (newFriend == nullptr)
    {
        foreach (shared_ptr<FsmTransition> neighboor, actualTargetState->getLogicState()->getOutgoingTransitions())
        {
            if ( (neighboor->getGraphicRepresentation() != nullptr) && (neighboor->getGraphicRepresentation() != this) ) // Needed, as in dynamic mode I still belong to states!
            {
                if (neighboor->getTarget() == actualSourceState->getLogicState())
                {
                    newFriend= neighboor;
                    break;
                }
            }
        }
    }

    if (newFriend != nullptr)
    {
        setNeighbors(newFriend->getGraphicRepresentation()->helloIMYourNewNeighbor());
    }
}

FsmGraphicTransitionNeighborhood* FsmGraphicTransition::helloIMYourNewNeighbor()
{
    if (neighbors == nullptr)
    {
        setNeighbors(new FsmGraphicTransitionNeighborhood(this->source, this->target));
        // Redraw useless (harmful?) as there is currenly only me in mates.
        // Redraw will be automatic on mate joining neighborhood
    }

    return neighbors;
}

void FsmGraphicTransition::setNeighbors(FsmGraphicTransitionNeighborhood *neighborhood)
{
    // Should not happen if already having one.
    // Maybe we should test it?
    this->neighbors = neighborhood;

    neighborhood->insertAndNotify(this);
    connect(this->neighbors, &FsmGraphicTransitionNeighborhood::contentChangedEvent, this, &FsmGraphicTransition::updateDisplay);

    treatSelectionBox();
}

void FsmGraphicTransition::quitNeighboorhood()
{
    // Algo is as follows:
    // If no mates => nothing to do!
    // Else quit neighborhood. This will notify remaning members,
    // and if only one remaining, it will disolve neighborhood

    if (neighbors != nullptr)
    {
        disconnect(this->neighbors, &FsmGraphicTransitionNeighborhood::contentChangedEvent, this, &FsmGraphicTransition::updateDisplay);
        neighbors->removeAndNotify(this);
        neighbors = nullptr;
    }
}

QGraphicsTextItem* FsmGraphicTransition::getConditionText() const
{
    return conditionText;
}

QGraphicsItemGroup* FsmGraphicTransition::getActionsBox() const
{
    return actionsBox;
}


void FsmGraphicTransition::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    if (currentMode == mode::standardMode)
    {
        ContextMenu* menu = new ContextMenu();
        menu->addTitle(tr("Transition"));

        menu->addAction(tr("Change source"));
        menu->addAction(tr("Change target"));
        menu->addAction(tr("Edit"));
        menu->addAction(tr("Delete"));
        menu->popup(event->screenPos());

        connect(menu, &QMenu::triggered, this, &FsmGraphicTransition::treatMenu);
    }
}

void FsmGraphicTransition::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Menu)
    {
        QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

        QGraphicsView* view = scene()->views()[0];

        QPoint posOnParent = view->mapFromScene(this->scenePos());

        QPoint posOnScreen = view->mapToGlobal(posOnParent);
        contextEvent->setScreenPos(posOnScreen);

        this->contextMenuEvent(contextEvent);
    }
    else if (event->key() == Qt::Key_Delete)
    {
        shared_ptr<FsmTransition> l_logicTransition = this->logicTransition.lock();

        if (l_logicTransition != nullptr)
            l_logicTransition->getSource()->removeOutgoingTransition(l_logicTransition);
    }
    else
    {
        event->ignore();
    }
}

QVariant FsmGraphicTransition::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedChange)
    {
        // If changing to selected
        if (value.toBool() == true)
        {
            // Refuse selection if there are other item(s) already selected
            if (this->scene()->selectedItems().count() != 0)
                return (QVariant)false;
        }
    }
    else if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged)
    {
        this->treatSelectionBox();
    }

    return QGraphicsItem::itemChange(change, value);

}

void FsmGraphicTransition::treatMenu(QAction* action)
{
    if (action->text() == tr("Change source"))
        emit dynamicSourceCalledEvent(this);
    else if (action->text() == tr("Change target"))
        emit dynamicTargetCalledEvent(this);
    if (action->text() == tr("Edit"))
        emit editCalledEvent(this->logicTransition.lock());
    else if (action->text() == tr("Delete"))
    {
        // This call will destroy the current object as consequence of the logic object destruction

        shared_ptr<FsmTransition> l_logicTransition = this->logicTransition.lock();

        if (l_logicTransition != nullptr)
            l_logicTransition->getSource()->removeOutgoingTransition(l_logicTransition);
    }

}

void FsmGraphicTransition::machineModeChangedEventHandler(Machine::mode)
{
    this->updateText();
}
