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
#include "fsmgraphicaltransition.h"

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QResizeEvent>
#include <QPainter>

// Debug
#include <QDebug>

// States classes
#include "fsmgraphicalstate.h"
#include "fsmstate.h"
#include "fsmscene.h"
#include "fsmtransition.h"
#include "fsmgraphicaltransitionneighborhood.h"
#include "signal.h"
#include "contextmenu.h"


qreal FsmGraphicalTransition::arrowEndSize = 10;
qreal FsmGraphicalTransition::middleBarLength = 20;
QPen FsmGraphicalTransition::selectionPen = QPen(Qt::DashLine);
QPen FsmGraphicalTransition::standardPen = QPen(Qt::SolidPattern, 3);
QPen FsmGraphicalTransition::editPen = QPen(QBrush(Qt::red, Qt::SolidPattern), 3);
QPen FsmGraphicalTransition::activePen = QPen(QBrush(Qt::darkGreen, Qt::SolidPattern), 3);
QPen FsmGraphicalTransition::inactivePen = QPen(QBrush(Qt::red, Qt::SolidPattern), 3);

QPixmap FsmGraphicalTransition::getPixmap(uint size)
{
    QPixmap pixmap(QSize(size, size));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    painter.setPen(FsmGraphicalTransition::standardPen);
    painter.drawLine(0, 0, size, size);
    painter.drawLine(0, 0, size/3, 0);
    painter.drawLine(0, 0, 0, size/3);

    return pixmap;
}

FsmGraphicalTransition::FsmGraphicalTransition()
{
    currentPen = &standardPen;

    conditionText = new QGraphicsTextItem();
    actionsBox    = new QGraphicsItemGroup();

    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsFocusable);
}

FsmGraphicalTransition::FsmGraphicalTransition(FsmGraphicalState* source, const QPointF& dynamicMousePosition) :
    FsmGraphicalTransition()
{
    currentMode = mode::initMode;
    this->setSourceState(source);
    this->setDynamicTargetMode(dynamicMousePosition);
    // We are now in dynamic target mode

    finishInitialize();
}

FsmGraphicalTransition::FsmGraphicalTransition(shared_ptr<FsmTransition> logicTransition) :
    FsmGraphicalTransition()
{
    logicTransition->setGraphicalRepresentation(this);
    connect(logicTransition.get(), &MachineActuatorComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicalTransition::updateText);
    connect(logicTransition.get(), &FsmTransition::componentDynamicStateChangedEvent,                   this, &FsmGraphicalTransition::updateText);

    currentMode = mode::initMode;
    this->setSourceState(logicTransition->getSource()->getGraphicalRepresentation());
    this->setTargetState(logicTransition->getTarget()->getGraphicalRepresentation());
    currentMode = mode::standardMode;

    this->logicalTransition = logicTransition;

    checkNeighboors();
    finishInitialize();
}

void FsmGraphicalTransition::finishInitialize()
{
    rebuildArrowEnd();
    updateText();
    updateDisplay();
}

FsmGraphicalTransition::~FsmGraphicalTransition()
{
    quitNeighboorhood();

    delete conditionText;
    delete actionsBox;

    shared_ptr<FsmTransition> transition = this->logicalTransition.lock();
    if (transition != nullptr)
        transition->clearGraphicalRepresentation();
}

void FsmGraphicalTransition::setLogicalTransition(shared_ptr<FsmTransition> transition)
{
    // This function can be called only once, we never reaffect a graphical transition
    if (this->logicalTransition.lock() == nullptr)
    {
        connect(transition.get(), &MachineActuatorComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicalTransition::updateText);
        connect(transition.get(), &FsmTransition::componentDynamicStateChangedEvent,                    this, &FsmGraphicalTransition::updateText);

        this->logicalTransition = transition;

        this->updateText();
    }
    else
        qDebug() << "(FSM Graphical Transition:) ERROR! Trying to reaffect logical transition while already linked! Ignored call.";
}

shared_ptr<FsmTransition> FsmGraphicalTransition::getLogicalTransition() const
{
    return logicalTransition.lock();
}

bool FsmGraphicalTransition::setSourceState(FsmGraphicalState* newSource)
{
    autoTransitionNeedsRedraw = true;

    if (currentMode == mode::initMode)
    {
        this->source = newSource;

        // Connect to new state
        connect(this->source, &FsmGraphicalState::stateMovingEvent, this, &FsmGraphicalTransition::updateDisplay);

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
                disconnect(this->source, &FsmGraphicalState::stateMovingEvent, this, &FsmGraphicalTransition::updateDisplay);

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newSource)
                quitNeighboorhood();

            this->source = newSource;

            // Connect to new state
            connect(this->source, &FsmGraphicalState::stateMovingEvent, this, &FsmGraphicalTransition::updateDisplay);

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

bool FsmGraphicalTransition::setTargetState(FsmGraphicalState* newTarget)
{
    autoTransitionNeedsRedraw = true;

    if (currentMode == mode::initMode)
    {
        this->target = newTarget;

        // Connect to new state
        connect(this->target, &FsmGraphicalState::stateMovingEvent, this, &FsmGraphicalTransition::updateDisplay);

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
                    disconnect(this->target, &FsmGraphicalState::stateMovingEvent, this, &FsmGraphicalTransition::updateDisplay);
            }

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newTarget)
                quitNeighboorhood();


            this->target = newTarget;

            // Connect to new state
            connect(this->target, &FsmGraphicalState::stateMovingEvent, this, &FsmGraphicalTransition::updateDisplay);

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

void FsmGraphicalTransition::setMousePosition(const QPointF& mousePos)
{
    this->dynamicState= nullptr;
    this->mousePosition = mousePos;

    quitNeighboorhood();
    updateDisplay();
}

FsmGraphicalState* FsmGraphicalTransition::getSource() const
{
    return this->source;
}

FsmGraphicalState* FsmGraphicalTransition::getTarget() const
{
    return this->target;
}

void FsmGraphicalTransition::rebuildArrowEnd()
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

void FsmGraphicalTransition::treatSelectionBox()
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
            qDebug() << "(Fsm graphical transition:) Error white treating selection box: unknown shape!";
    }
}

void FsmGraphicalTransition::updateText()
{
    shared_ptr<FsmTransition> transition = this->logicalTransition.lock();
    //
    // Condition

    // Should also make background semi-transparent... (we could avoid color?)
    if (transition != nullptr)
    {
        if ( (scene() != nullptr) && (((FsmScene*)scene())->getMode() == ResourceBar::mode::simulateMode) )
        {
            if (transition->getCondition() == nullptr)
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");
            else
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + transition->getCondition()->getText(true) + "</div>");
            if (conditionLine != nullptr)
            {
                if (transition->getCondition()->isTrue())
                    conditionLine->setPen(activePen);
                else
                    conditionLine->setPen(inactivePen);
            }
        }
        else
        {
            if (transition->getCondition() == nullptr)
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");
            else
                conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + transition->getCondition()->getText() + "</div>");
            if (conditionLine != nullptr)
                conditionLine->setPen(standardPen);
        }
    }
    else
        conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");

    //
    // Actions

    if (transition != nullptr)
    {
        qDeleteAll(actionsBox->childItems());
        actionsBox->childItems().clear();

        QList<shared_ptr<Signal>> actions = transition->getActions();

        qreal maxTextWidth = 0;


        for (int i = 0 ; i < actions.count() ; i++)
        {
            QGraphicsTextItem* actionText = new QGraphicsTextItem(actions[i]->getName(), actionsBox);

            QString currentActionText;

            if ( (scene() != nullptr) && (((FsmScene*)scene())->getMode() == ResourceBar::mode::simulateMode) )
                currentActionText = actions[i]->getText(true);
            else
                currentActionText = actions[i]->getText(false);


            if (transition->getActionType(actions[i]) == MachineActuatorComponent::action_types::set)
                currentActionText += " = 1";
            else if (transition->getActionType(actions[i]) == MachineActuatorComponent::action_types::reset)
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

void FsmGraphicalTransition::updateDisplay()
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
    FsmGraphicalState* currentSourceState = nullptr;
    FsmGraphicalState* currentTargetState = nullptr;

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

    delete this->conditionLine;
    this->conditionLine = nullptr;

    if ( (neighbors == nullptr) && (currentSourceState != currentTargetState) )
    {
        delete arrowBody;
        arrowBody = nullptr;

        // If no mates, draw a straight line

        QLineF straightLine(QPointF(0, 0), currentTargetPoint - currentSourcePoint);

        // Set line appropriate size (reduce current size by correct number of state radius to point on states border)
        if ( ((currentMode == mode::dynamicSourceMode) || (currentMode == mode::dynamicTargetMode)) && (dynamicState == nullptr) )
        {
            // Reduce line length to match space between state center and mouse position
            straightLine.setLength(straightLine.length() - FsmGraphicalState::getRadius());
        }
        else
        {
            // Reduce line length to match space between states
            straightLine.setLength(straightLine.length() - 2*FsmGraphicalState::getRadius());
        }

        // Create line graphical representation
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
            translationVector.setLength(FsmGraphicalState::getRadius());

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
            delete arrowBody;
            arrowBody = nullptr;

            uint rank;
            if (neighbors != nullptr)
                rank = neighbors->whatIsMyRank(this);
            else
                rank = 0;

            QLineF stateCenterToArcStartVector(0, 0, 1, -1);
            stateCenterToArcStartVector.setLength(FsmGraphicalState::getRadius());

            QLineF stateCenterToArcEndVector(0, 0, -1, -1);
            stateCenterToArcEndVector.setLength(FsmGraphicalState::getRadius());

            qreal unitaryArcWidth  = 2*FsmGraphicalState::getRadius();
            qreal unitaryArcHeight = 2*FsmGraphicalState::getRadius();

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
        delete arrowBody;
        arrowBody = nullptr;

        // Take mates in consideration to draw a curve
        // All mates will have a curved body, which curve intensity
        // depends on their rank in mates

        // Get actual source and target in mates's coordinate system
        if (((currentMode == mode::standardMode)      && (neighbors->getSource() != this->source)) ||
                ((currentMode == mode::dynamicTargetMode) && (neighbors->getSource() != this->source)) ||
                ((currentMode == mode::dynamicSourceMode) && (neighbors->getSource() != this->dynamicState)))
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

bool FsmGraphicalTransition::setDynamicSourceMode(const QPointF& mousePosition)
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

bool FsmGraphicalTransition::setDynamicTargetMode(const QPointF& mousePosition)
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

bool FsmGraphicalTransition::endDynamicMode(bool keepChanges)
{
    shared_ptr<FsmTransition> transition = this->logicalTransition.lock();
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
            if (transition != nullptr)
            {
                shared_ptr<FsmState> oldOwner = transition->getSource();
                shared_ptr<FsmState> newOwner = dynamicState->getLogicalState();

                transition->setSource(newOwner);
                oldOwner->removeOutgoingTransition(transition);
                newOwner->addOutgoingTransition(transition);
            }
        }
        else if (currentMode == mode::dynamicTargetMode)
        {
            currentMode = mode::standardMode;
            setTargetState(dynamicState);
            if (transition != nullptr)
            {
                shared_ptr<FsmState> oldTarget = transition->getTarget();
                shared_ptr<FsmState> newTarget = dynamicState->getLogicalState();

                transition->setTarget(newTarget);
                oldTarget->removeIncomingTransition(transition);
                newTarget->addIncomingTransition(transition);
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

void FsmGraphicalTransition::checkNeighboors()
{
    // Algo is as follows:
    // Check all outcoming transitions from both ends' states (except me)
    // If none => all right, no precautions needed
    // If 1 => initiate a mates secrete society
    // If more than one => join mates secrete society

    FsmGraphicalState* actualSourceState;
    FsmGraphicalState* actualTargetState;

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

    foreach (shared_ptr<FsmTransition> neighboor, actualSourceState->getLogicalState()->getOutgoingTransitions())
    {
        if ( (neighboor->getGraphicalRepresentation() != nullptr) && (neighboor->getGraphicalRepresentation() != this) ) // Needed, as in dynamic mode I still belong to states!
        {
            if (neighboor->getTarget() == actualTargetState->getLogicalState())
            {
                newFriend = neighboor;
                break;
            }
        }
    }

    if (newFriend == nullptr)
    {
        foreach (shared_ptr<FsmTransition> neighboor, actualTargetState->getLogicalState()->getOutgoingTransitions())
        {
            if ( (neighboor->getGraphicalRepresentation() != nullptr) && (neighboor->getGraphicalRepresentation() != this) ) // Needed, as in dynamic mode I still belong to states!
            {
                if (neighboor->getTarget() == actualSourceState->getLogicalState())
                {
                    newFriend= neighboor;
                    break;
                }
            }
        }
    }

    if (newFriend != nullptr)
    {
        setNeighbors(newFriend->getGraphicalRepresentation()->helloIMYourNewNeighbor());
    }
}

FsmGraphicalTransitionNeighborhood* FsmGraphicalTransition::helloIMYourNewNeighbor()
{
    if (neighbors == nullptr)
    {
        setNeighbors(new FsmGraphicalTransitionNeighborhood(this->source, this->target));
        // Redraw useless (harmful?) as there is currenly only me in mates.
        // Redraw will be automatic on mate joining neighborhood
    }

    return neighbors;
}

void FsmGraphicalTransition::setNeighbors(FsmGraphicalTransitionNeighborhood *neighborhood)
{
    // Should not happen if already having one.
    // Maybe we should test it?
    this->neighbors = neighborhood;

    neighborhood->insertAndNotify(this);
    connect(this->neighbors, &FsmGraphicalTransitionNeighborhood::contentChangedEvent, this, &FsmGraphicalTransition::updateDisplay);

    treatSelectionBox();
}

void FsmGraphicalTransition::quitNeighboorhood()
{
    // Algo is as follows:
    // If no mates => nothing to do!
    // Else quit neighborhood. This will notify remaning members,
    // and if only one remaining, it will disolve neighborhood

    if (neighbors != nullptr)
    {
        disconnect(this->neighbors, &FsmGraphicalTransitionNeighborhood::contentChangedEvent, this, &FsmGraphicalTransition::updateDisplay);
        neighbors->removeAndNotify(this);
        neighbors = nullptr;
    }
}

QGraphicsTextItem* FsmGraphicalTransition::getConditionText() const
{
    return conditionText;
}

QGraphicsItemGroup* FsmGraphicalTransition::getActionsBox() const
{
    return actionsBox;
}


void FsmGraphicalTransition::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
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

        connect(menu, &QMenu::triggered, this, &FsmGraphicalTransition::treatMenu);
        connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    }
}

void FsmGraphicalTransition::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        shared_ptr<FsmTransition> transition = this->logicalTransition.lock();
        transition->getSource()->removeOutgoingTransition(transition);
    }
    else
    {
        event->ignore();
    }
}

void FsmGraphicalTransition::treatMenu(QAction* action)
{
    if (action->text() == tr("Change source"))
        emit dynamicSourceCalledEvent(this);
    else if (action->text() == tr("Change target"))
        emit dynamicTargetCalledEvent(this);
    if (action->text() == tr("Edit"))
        emit editCalledEvent(logicalTransition.lock());
    else if (action->text() == tr("Delete"))
    {
        // This call will destroy the current object as consequence of the logical object destruction
        shared_ptr<FsmTransition> transition = this->logicalTransition.lock();
        transition->getSource()->removeOutgoingTransition(transition);
    }

}
