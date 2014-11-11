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

#include <QDebug>
#include <QMouseEvent>

#include "fsmscene.h"

#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"
#include "fsmtools.h"
#include "fsm.h"
#include "scenewidget.h"
#include "machine.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "resourcesbar.h"

FsmScene::FsmScene(ResourcesBar* resources, Fsm* machine) :
    GenericScene(resources)
{
    this->machine = machine;
    connect(this, SIGNAL(selectionChanged()), this, SLOT(handleSelection()));

    foreach(FsmState* state, ((Fsm*)machine)->getStates())
    {
        addState(state, state->position);
    }

    foreach(FsmTransition* transition, ((Fsm*)machine)->getTransitions())
    {
        FsmGraphicalTransition* graphicalTransition = new FsmGraphicalTransition(transition);
        addTransition(graphicalTransition);
    }
}

FsmScene::~FsmScene()
{
    // We have to delete transitions first, so handle
    // item deletion manually

    QList<FsmGraphicalTransition*> transitions;
    QList<FsmGraphicalState*> states;

    foreach(QGraphicsItem* item, items())
    {
        FsmGraphicalTransition* currentTransition = dynamic_cast<FsmGraphicalTransition*>(item);

        if (currentTransition != nullptr)
            transitions.append(currentTransition);
    }

    foreach(QGraphicsItem* item, items())
    {
        FsmGraphicalState* currentState = dynamic_cast<FsmGraphicalState*>(item);

        if (currentState!= nullptr)
            states.append(currentState);
    }

    qDeleteAll(transitions);
    qDeleteAll(states);
}

ResourcesBar::mode FsmScene::getMode() const
{
    return resources->getCurrentMode();
}

void FsmScene::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    if (me->button() == Qt::LeftButton)
    {
        if ((!isEditingTransitionSource) && (!isEditingTransitionTarget)) // Means normal mode, as isDrawingTransition can't have a click (wait for button release)
        {
            MachineTools::tool currentTool = resources->getBuildTools()->getTool();

            if (currentTool == MachineTools::tool::state)
            {
                // Create logical state
                FsmState* logicState = new FsmState(machine);

                // Create graphical state
                addState(logicState, me->scenePos());

                // Maintains state under mouse until button is relased
                GenericScene::mousePressEvent(me);
            }
            else if (currentTool == FsmTools::tool::initial_state)
            {
                // Create logical state
                FsmState* logicState = new FsmState(machine);

                // Create graphical state
                addState(logicState, me->scenePos());

                // Set state initial
                logicState->setInitial();

                // Maintains state under mouse until button is relased
                GenericScene::mousePressEvent(me);
            }
            else if (currentTool == FsmTools::tool::transition)
            {

                FsmGraphicalState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

                if ( stateUnderMouse != nullptr)
                {
                    isDrawingTransition = true;
                    currentTransition = new FsmGraphicalTransition(stateUnderMouse, me->scenePos());
                    currentTransition->setMousePosition(QPointF(me->scenePos()));

                    addTransition(currentTransition);

                }
            }
            else if (currentTool == FsmTools::tool::none)
            {
                // If no tool, this is selection mode.
                GenericScene::mousePressEvent(me);

                // Update transitions selection box as it is a manual box
                foreach(QGraphicsItem* item, items())
                {
                    FsmGraphicalTransition* currentTransition = dynamic_cast<FsmGraphicalTransition*>(item);

                    if (currentTransition != nullptr)
                        currentTransition->treatSelectionBox();
                }

            }
            else
            {
                qDebug() << "(FSM display:) Unsupported tool!";
                GenericScene::mousePressEvent(me);
            }
        }
        else if (isEditingTransitionSource || isEditingTransitionTarget)
        {
            FsmGraphicalState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));
            if (stateUnderMouse != nullptr)
                currentTransition->endDynamicMode(true);
            else
                currentTransition->endDynamicMode(false);

            isEditingTransitionSource = false;
            isEditingTransitionTarget = false;
            currentTransition = nullptr;
        }
    }
    else if (me->button() == Qt::RightButton)
    {
        // Ignore right-click while a transition is in dynamic mode (do not deselect current tool)
        if ( ( (!isDrawingTransition)        &&
               (!isEditingTransitionSource)  &&
               (!isEditingTransitionTarget)
               ) &&
             // and also if there is an item under mouse (this is a context menu event)
             (this->items(me->scenePos(), Qt::IntersectsItemShape, Qt::DescendingOrder).count() == 0)
             )
        {
            GenericScene::mousePressEvent(me);
        }
        else if (isEditingTransitionSource || isEditingTransitionTarget)
        {
            // Cancel transition edition
            isEditingTransitionSource = false;
            isEditingTransitionTarget = false;
            currentTransition->endDynamicMode(false);
            currentTransition = nullptr;
        }
    }
    else
        GenericScene::mousePressEvent(me);

}

void FsmScene::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    static FsmGraphicalState* previousStatePointed = nullptr;

    if (isDrawingTransition || isEditingTransitionSource || isEditingTransitionTarget)
    {
        FsmGraphicalState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

        if (stateUnderMouse != nullptr)
        {
            if (isDrawingTransition || isEditingTransitionTarget)
            {
                if (stateUnderMouse != previousStatePointed)
                {
                    currentTransition->setTargetState(stateUnderMouse);
                    previousStatePointed = stateUnderMouse;
                }
            }
            else
            {

                if (stateUnderMouse != previousStatePointed)
                {
                    currentTransition->setSourceState(stateUnderMouse);
                    previousStatePointed = stateUnderMouse;
                }
            }
        }
        else
        {
            currentTransition->setMousePosition(me->scenePos());
            previousStatePointed = nullptr;
        }
    }
    //else if (resources->getBuildTools()->getTool() == MachineTools::state)
    //{
    /*if (ghostState == nullptr)
        {
            ghostState = new FsmGraphicalState();
            addItem(ghostState);
        }

        ghostState->setPos(me->scenePos());*/

    // Should change cursor instead
    //}
    else
    {
        GenericScene::mouseMoveEvent(me);
    }
}

void FsmScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* me)
{
    if (me->button() == Qt::LeftButton)
    {
        if (isDrawingTransition)
        {
            FsmGraphicalState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

            // Do not allow same-state transition while no representation available
            if ((stateUnderMouse != nullptr) /*&& (stateUnderMouse != currentTransition->getSource())*/)
            {
                currentTransition->setTargetState(stateUnderMouse);
                currentTransition->endDynamicMode(true);

                FsmTransition* logicTransition = new FsmTransition(machine, currentTransition->getSource()->getLogicalState(), currentTransition->getTarget()->getLogicalState(), currentTransition);
                currentTransition->setLogicalTransition(logicTransition);
            }
            else
                delete currentTransition;

            currentTransition = nullptr;
            isDrawingTransition = false;
        }
        else
            GenericScene::mouseReleaseEvent(me);
    }
    else
        GenericScene::mouseReleaseEvent(me);

    updateSceneRect();
}

void FsmScene::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        if (isDrawingTransition)
        {
            isDrawingTransition = false;
            delete currentTransition;
            currentTransition = nullptr;
        }
        else if (isEditingTransitionSource || isEditingTransitionTarget)
        {
            // Cancel transition edition
            isEditingTransitionSource = false;
            isEditingTransitionTarget = false;
            currentTransition->endDynamicMode(false);
            currentTransition = nullptr;
        }
    }
    if (this->selectedItems().count() == 1)
    {
        // Handle deletion applying to dynamic transition
        if (isDrawingTransition || isEditingTransitionSource || isEditingTransitionTarget)
        {
            // We have no knowledge of your doings
            isDrawingTransition = false;
            isEditingTransitionSource = false;
            isEditingTransitionTarget = false;
            currentTransition = nullptr;

            // Transmit deletion order
            GenericScene::keyPressEvent(event);
        }
        else
            GenericScene::keyPressEvent(event);
    }
    else
    {
        // Only transmit move orders to states when multiple childs
        if ( (event->key() == Qt::Key_Right) ||
             (event->key() == Qt::Key_Left) ||
             (event->key() == Qt::Key_Up) ||
             (event->key() == Qt::Key_Down)
           )
        {
            foreach (QGraphicsItem* item, this->selectedItems())
            {
                FsmGraphicalState* state = dynamic_cast<FsmGraphicalState*>(item);

                if (state != nullptr)
                    state->keyPressEvent(event);
            }
        }
    }
}

void FsmScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Do not show menus while a transition is in dynamic mode
    if ((!isDrawingTransition) &&
            (!isEditingTransitionSource) &&
            (!isEditingTransitionTarget))
        GenericScene::contextMenuEvent(event);
}

void FsmScene::transitionCallsDynamicSource(FsmGraphicalTransition* transition)
{
    isEditingTransitionSource = true;
    currentTransition = transition;

    clearSelection();
    transition->setSelected(true);

    currentTransition->setDynamicSourceMode(QPointF());
}

void FsmScene::transitionCallsDynamicTarget(FsmGraphicalTransition* transition)
{
    isEditingTransitionTarget = true;
    currentTransition = transition;

    clearSelection();
    transition->setSelected(true);

    currentTransition->setDynamicTargetMode(QPointF());
}

void FsmScene::handleSelection()
{
    if (this->selectedItems().count() == 1)
        this->selectedItems()[0]->setFocus();
    else
        this->clearFocus();

    if (this->selectedItems().count() == 1)
    {
        FsmGraphicalState* currentState = dynamic_cast< FsmGraphicalState* >(this->selectedItems()[0]);
        if (currentState != nullptr)
        {
            resources->selectedState(currentState->getLogicalState());
        }
        else
        {
            FsmGraphicalTransition* currentTransition= dynamic_cast< FsmGraphicalTransition* >(this->selectedItems()[0]);
            if (currentTransition!= nullptr)
            {
                resources->selectedTransition(currentTransition->getLogicalTransition());
            }
        }
    }
    else
    {
        resources->clearSelection();
    }
}

FsmGraphicalState* FsmScene::getStateAt(const QPointF& location) const
{
    QList<QGraphicsItem *>  itemsAtThisPoint = this->items(location, Qt::IntersectsItemShape, Qt::DescendingOrder); //this->((SceneWidget*)parent())->transform());
    // Warning: if using transform on view, the upper line should be adapted!


    foreach( QGraphicsItem* item, itemsAtThisPoint)
    {
        // Select the topmost visible state
        FsmGraphicalState* currentItem = dynamic_cast<FsmGraphicalState*> (item);

        if ( currentItem != nullptr)
            return currentItem;
    }

    return nullptr;
}


void FsmScene::setDisplaySize(const QSize& newSize)
{
    this->displaySize = newSize;
    updateSceneRect();
}

void FsmScene::simulationModeChanged()
{
    if (resources->getCurrentMode() == ResourcesBar::mode::editMode)
    {
        handleSelection();
    }
}

void FsmScene::updateSceneRect()
{
    // This function is called on resize, and
    // When mouse button release (can be item moved).
    // Not dynamically linked to item moving event,
    // as it caused too much difficulty with asynchonous
    // events. Mutex did not good as it desynchonized item
    // position and mouse position.


    // Scene size is at least size of the display view,
    // But can be bigger is machine is bigger

    // Initially, set scene to be size of display, centered on point 0
    // As this is this base view that will be extended,
    // Point (0, 0) will always be part of displayed scene.
    // This is to avoid moving a single state and dynamically recalculating
    // rect to center on it... Quickly comes to high position values.
    QRect baseDisplayRectangle(QPoint(-displaySize.width()/2, -displaySize.height()/2), displaySize);


    // Then adjust to include items not seen (includes a margin)
    int leftmostPosition = baseDisplayRectangle.topLeft().x();
    int topmostPosition = baseDisplayRectangle.topLeft().y();
    int rightmostPosition = baseDisplayRectangle.bottomRight().x();
    int bottommostPosition = baseDisplayRectangle.bottomRight().y();

    int margin = 100;

    foreach(QGraphicsItem* item, this->items())
    {
        // Get item's boundig box scene coordinates
        // Use two point instead of the rect to avoid polygon conversion
        QPointF itemTopLeftF = item->mapToScene(item->boundingRect().topLeft());
        QPoint itemTopLeft = itemTopLeftF.toPoint();
        QPointF itemBottomRightF = item->mapToScene(item->boundingRect().bottomRight());
        QPoint itemBottomRight = itemBottomRightF.toPoint();

        if (itemTopLeft.x()-margin < leftmostPosition)
            leftmostPosition = itemTopLeft.x()-margin;

        if (itemTopLeft.y()-margin < topmostPosition)
            topmostPosition = itemTopLeft.y()-margin;

        if (itemBottomRight.x()+margin > rightmostPosition)
            rightmostPosition = itemBottomRight.x()+margin;

        if (itemBottomRight.y()+margin > bottommostPosition)
            bottommostPosition = itemBottomRight.y()+margin;
    }

    QRect finalDisplayRectangle(QPoint(leftmostPosition, topmostPosition), QPoint(rightmostPosition, bottommostPosition));

    this->setSceneRect(finalDisplayRectangle);
}

void FsmScene::stateCallsEdit(FsmState* state)
{
    clearSelection();
    state->getGraphicalRepresentation()->setSelected(true);
    resources->selectedState(state, true);
}

void FsmScene::stateCallsRename(FsmState* state)
{
    clearSelection();
    state->getGraphicalRepresentation()->setSelected(true);
    resources->selectedState(state, true, true);
}

void FsmScene::transitionCallsEdit(FsmTransition* transition)
{
    clearSelection();
    transition->getGraphicalRepresentation()->setSelected(true);
    resources->selectedTransition(transition, true);
}

FsmGraphicalState* FsmScene::addState(FsmState* logicState, QPointF location)
{
    // Create graphical state
    FsmGraphicalState *newState = new FsmGraphicalState(logicState);
    addItem(newState);
    newState->setPos(location);
    newState->setZValue(1);

    QGraphicsItemGroup* actionsBox = newState->getActionsBox();
    addItem(actionsBox);
    actionsBox->setZValue(3);

    connect(newState, SIGNAL(callEdit(FsmState*)), this, SLOT(stateCallsEdit(FsmState*)));
    connect(newState, SIGNAL(callRename(FsmState*)), this, SLOT(stateCallsRename(FsmState*)));

    return newState;
}

void FsmScene::addTransition(FsmGraphicalTransition* newTransition)
{
    connect(newTransition, SIGNAL(callDynamicSource(FsmGraphicalTransition*)), this, SLOT(transitionCallsDynamicSource(FsmGraphicalTransition*)));
    connect(newTransition, SIGNAL(callDynamicTarget(FsmGraphicalTransition*)), this, SLOT(transitionCallsDynamicTarget(FsmGraphicalTransition*)));
    connect(newTransition, SIGNAL(callEdit(FsmTransition*)), this, SLOT(transitionCallsEdit(FsmTransition*)));

    addItem(newTransition);
    newTransition->setZValue(2);

    QGraphicsTextItem* condition = newTransition->getConditionText();
    addItem(condition);
    condition->setZValue(3);

    QGraphicsItemGroup* actionsBox = newTransition->getActionsBox();
    addItem(actionsBox);
    actionsBox->setZValue(3);
}
