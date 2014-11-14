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

#include <QGraphicsSceneMouseEvent>
#include <QPointF>
#include <QGraphicsLineItem>
#include <QBrush>
#include <QList>
#include <QPainterPath>
#include <QGraphicsSceneContextMenuEvent>
#include <QLabel>
#include <QWidgetAction>
#include <QResizeEvent>

#include <QDebug>

#include "fsmgraphicaltransition.h"

#include "fsmgraphicalstate.h"
#include "fsmstate.h"
#include "fsmscene.h"
#include "fsmtransition.h"
#include "fsmgraphicaltransitionneighborhood.h"
#include "logicvariable.h"
#include "logicequation.h"
#include "contextmenu.h"

qreal FsmGraphicalTransition::arrowEndSize = 10;
qreal FsmGraphicalTransition::middleBarLength = 20;
QPen FsmGraphicalTransition::standardPen = QPen(Qt::SolidPattern, 3);
QPen FsmGraphicalTransition::editPen = QPen(QBrush(Qt::red, Qt::SolidPattern), 3);
QPen FsmGraphicalTransition::selectionPen = QPen(Qt::DashLine);

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

FsmGraphicalTransition::FsmGraphicalTransition(FsmTransition* logicTransition) :
    FsmGraphicalTransition()
{
    this->logicalTransition = logicTransition;
    this->logicalTransition->setGraphicalRepresentation(this);
    connect(this->logicalTransition, SIGNAL(transitionConfigurationChanged()), this, SLOT(updateText()));

    currentMode = mode::initMode;
    this->setSourceState(this->logicalTransition->getSource()->getGraphicalRepresentation());
    this->setTargetState(this->logicalTransition->getTarget()->getGraphicalRepresentation());
    currentMode = mode::standardMode;

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

    if (logicalTransition != nullptr)
        logicalTransition->clearGraphicalRepresentation();
}

void FsmGraphicalTransition::setLogicalTransition(FsmTransition* transition)
{
    // This function can be called only once, we never reaffect a graphical transition
    if (this->logicalTransition == nullptr)
    {
        this->logicalTransition = transition;
        connect(this->logicalTransition, SIGNAL(transitionConfigurationChanged()), this, SLOT(updateText()));
        this->updateText();
    }
    else
        qDebug() << "(FSM Graphical Transition:) ERROR! Trying to reaffect logical transition while already linked! Ignored call.";
}

FsmTransition* FsmGraphicalTransition::getLogicalTransition() const
{
    return logicalTransition;
}

bool FsmGraphicalTransition::setSourceState(FsmGraphicalState* newSource)
{
    autoTransitionNeedsRedraw = true;

    if (currentMode == mode::initMode)
    {
        this->source = newSource;

        // Connect to new state
        connect(this->source, SIGNAL(moving()), this, SLOT(updateDisplay()));

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
                disconnect(this->source, SIGNAL(moving()), this, SLOT(updateDisplay()));

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newSource)
                quitNeighboorhood();

            this->source = newSource;

            // Connect to new state
            connect(this->source, SIGNAL(moving()), this, SLOT(updateDisplay()));

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
        connect(this->target, SIGNAL(moving()), this, SLOT(updateDisplay()));

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
                    disconnect(this->target, SIGNAL(moving()), this, SLOT(updateDisplay()));
            }

            // If dynamic target state exists, we are already in the right neighborhood
            if (dynamicState != newTarget)
                quitNeighboorhood();


            this->target = newTarget;

            // Connect to new state
            connect(this->target, SIGNAL(moving()), this, SLOT(updateDisplay()));

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
    //
    // Condition

    // Should also make background semi-transparent... (we could avoid color?)
    if (logicalTransition != nullptr)
    {
        if ( (scene() != nullptr) && (((FsmScene*)scene())->getMode() == ResourcesBar::mode::simulateMode) )
        {
            if (logicalTransition->getCondition()->isActive())
                conditionText->setHtml("<div style='background-color:#E8E8E8; color:green;'>" + logicalTransition->getCondition()->getText() + "</div>");
            else
                conditionText->setHtml("<div style='background-color:#E8E8E8; color:red;'>" + logicalTransition->getCondition()->getText() + "</div>");
        }
        else
        {
            conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + logicalTransition->getCondition()->getText() + "</div>");
        }
    }
    else
        conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");

    //
    // Actions

    if (logicalTransition != nullptr)
    {
        qDeleteAll(actionsBox->childItems());
        actionsBox->childItems().clear();

        QList<LogicVariable *> actions = logicalTransition->getActions();

        qreal maxTextWidth = 0;


        for (int i = 0 ; i < actions.count() ; i++)
        {
            QGraphicsTextItem* actionText = new QGraphicsTextItem(actions[i]->getName(), actionsBox);

            if (scene() != nullptr)
            {
                if (((FsmScene*)scene())->getMode() == ResourcesBar::mode::simulateMode)
                {
                    actionText = new QGraphicsTextItem("", actionsBox);

                    if (actions[i]->isActive())
                    {
                        actionText->setHtml("<div style='color:green;'>" + actions[i]->getName() + "</div>");
                    }
                    else
                    {
                        actionText->setHtml("<div style='color:red;'>" + actions[i]->getName() + "</div>");
                    }
                }
            }

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
            disconnect(neighbors, SIGNAL(contentChangedEvent()), this, SLOT(updateDisplay()));
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
        QLineF conditionLine = straightLine.normalVector();
        conditionLine.setLength(middleBarLength);
        QGraphicsLineItem *conditionLineDisplay = new QGraphicsLineItem(conditionLine, line);
        conditionLineDisplay->setPen(*currentPen);
        curveMiddle = QPointF(straightLine.p2()/2);
        conditionLineDisplay->setPos(curveMiddle - conditionLine.p2()/2);

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

            qreal rank;
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

            QLineF conditionLine(0, 0, 0, 1);
            conditionLine.setLength(middleBarLength);
            QGraphicsLineItem *conditionLineDisplay = new QGraphicsLineItem(conditionLine, arcItem);
            conditionLineDisplay->setPen(*currentPen);

            arcMiddle = QPointF(0, stateCenterToArcCenterVector.p2().y()-arcHeight/2);
            conditionLineDisplay->setPos(arcMiddle - conditionLine.p2()/2);

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
        arrowBody = neighbors->buildMyBody(currentPen, this, deltaCurveOrigin, curveMiddle, curveTarget, endAngle1, endAngle2);
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
            if (logicalTransition!=nullptr)
                logicalTransition->setSource(dynamicState->getLogicalState());
        }
        else if (currentMode == mode::dynamicTargetMode)
        {
            currentMode = mode::standardMode;
            setTargetState(dynamicState);
            if (logicalTransition!=nullptr)
                logicalTransition->setTarget(dynamicState->getLogicalState());
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

    FsmTransition* newFriend = nullptr;

    foreach (FsmTransition* neighboor, actualSourceState->getLogicalState()->getOutgoingTransitions())
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
        foreach (FsmTransition* neighboor, actualTargetState->getLogicalState()->getOutgoingTransitions())
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

void FsmGraphicalTransition::setNeighbors(FsmGraphicalTransitionNeighborhood *mates)
{
    this->neighbors = mates;

    mates->insertAndNotify(this);
    connect(mates, SIGNAL(contentChangedEvent()), this, SLOT(updateDisplay()));

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
        disconnect(neighbors, SIGNAL(contentChangedEvent()), this, SLOT(updateDisplay()));
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

        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(treatMenu(QAction*)));
    }
}

void FsmGraphicalTransition::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
        delete logicalTransition;
}

void FsmGraphicalTransition::treatMenu(QAction* action)
{
    if (action->text() == tr("Change source"))
        emit callDynamicSource(this);
    else if (action->text() == tr("Change target"))
        emit callDynamicTarget(this);
    if (action->text() == tr("Edit"))
        emit callEdit(logicalTransition);
    else if (action->text() == tr("Delete"))
    {
        // This call will destroy the current object as consequence of the logical object destruction
        delete logicalTransition;
    }

}
