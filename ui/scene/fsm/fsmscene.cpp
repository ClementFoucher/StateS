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
#include "fsmscene.h"

// Qt classes
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>

// Debug
#include <QDebug>

// StateS classes
#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"
#include "fsmtools.h"
#include "fsm.h"
#include "scenewidget.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "contextmenu.h"


FsmScene::FsmScene(shared_ptr<Fsm> machine, ResourceBar* resources) :
    GenericScene(resources)
{
    this->machine = machine;
    connect(this, &QGraphicsScene::selectionChanged, this, &FsmScene::handleSelection);

    foreach(shared_ptr<FsmState> state, machine->getStates())
    {
        addState(state, state->position);
    }

    foreach(shared_ptr<FsmTransition> transition, machine->getTransitions())
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

ResourceBar::mode FsmScene::getMode() const
{
    return resources->getCurrentMode();
}

void FsmScene::beginDrawTransition(FsmGraphicalState *source, const QPointF& currentMousePos)
{
    isDrawingTransition = true;

    if (!currentMousePos.isNull())
        currentTransition = new FsmGraphicalTransition(source, currentMousePos);
    else
    {
        // Compute mouse pos wrt. scene
        QPointF sceneMousePos = views()[0]->mapToScene(this->views()[0]->mapFromGlobal(QCursor::pos()));
        currentTransition = new FsmGraphicalTransition(source, sceneMousePos);
    }

    addTransition(currentTransition);
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
                shared_ptr<FsmState> logicState = machine->addState();

                // Create graphical state
                addState(logicState, me->scenePos());

                // Maintains state under mouse until button is relased
                GenericScene::mousePressEvent(me);
            }
            else if (currentTool == FsmTools::tool::initial_state)
            {
                // Create logical state
                shared_ptr<FsmState> logicState = machine->addState();

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
                    beginDrawTransition(stateUnderMouse, me->scenePos());
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
            // If there is a tool selected: unselect it
            if (this->resources->getBuildTools()->getTool() != MachineTools::tool::none)
            {
                this->resources->getBuildTools()->setTool(MachineTools::tool::none);

                GenericScene::mousePressEvent(me);
            }
            else
            {
                // Display context menu
                if (!this->isSimulating) // Adding state is not allowed in simulation mode
                {
                    ContextMenu* menu = new ContextMenu();
                    menu->addAction(tr("Add state"));
                    menu->addAction(tr("Add initial state"));
                    menu->popup(me->screenPos());

                    this->mousePos = me->scenePos();

                    connect(menu, &QMenu::triggered, this, &FsmScene::treatMenu);
                }
            }
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
        else if (isDrawingTransition)
        {
            isDrawingTransition = false;
            delete currentTransition;
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

                shared_ptr<FsmState> source = currentTransition->getSource()->getLogicalState();
                shared_ptr<FsmState> target = currentTransition->getTarget()->getLogicalState();

                shared_ptr<FsmTransition> logicTransition(new FsmTransition(machine, source, target, nullptr, currentTransition));
                source->addOutgoingTransition(logicTransition);
                target->addIncomingTransition(logicTransition);
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
        else
        {
            // Transmit to scene objects
            GenericScene::keyPressEvent(event);
        }
    }
    else if ( (event->key() == Qt::Key_Right) ||
              (event->key() == Qt::Key_Left)  ||
              (event->key() == Qt::Key_Up)    ||
              (event->key() == Qt::Key_Down)
            )
    {
        if (this->selectedItems().count() == 0)
        {
            // Ignore event => transmitted to view widget
            // which will handle it as view move
            event->ignore();
        }
        else
        {
            bool atLeastOneState = false;
            // Transmit event to each state in the list
            foreach (QGraphicsItem* item, this->selectedItems())
            {
                FsmGraphicalState* state = dynamic_cast<FsmGraphicalState*>(item);

                if (state != nullptr)
                {
                    atLeastOneState = true;
                    state->keyPressEvent(event);
                }
            }

            if (!atLeastOneState)
            {
                // Same as if there were no selected items at all
                event->ignore();
            }
        }
    }
    else
    {
        // All other events are passed directly to items
        GenericScene::keyPressEvent(event);
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

void FsmScene::transitionCallsDynamicSourceEventHandler(FsmGraphicalTransition* transition)
{
    isEditingTransitionSource = true;
    currentTransition = transition;

    clearSelection();
    transition->setSelected(true);

    currentTransition->setDynamicSourceMode(QPointF());
}

void FsmScene::transitionCallsDynamicTargetEventHandler(FsmGraphicalTransition* transition)
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
    if (resources->getCurrentMode() == ResourceBar::mode::editMode)
    {
        handleSelection();
        this->isSimulating = false;
    }
    else if (resources->getCurrentMode() == ResourceBar::mode::simulateMode)
    {
        this->isSimulating = true;
    }
}

void FsmScene::updateSceneRect()
{
    // This function is called on resize, and
    // When mouse button release (can be item moved).
    // Not dynamically linked to item moving event,
    // as it caused too much difficulty with asynchonous
    // events. Mutex did no good as it desynchonized item
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
        // Use two points instead of the rect to avoid polygon conversion
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

void FsmScene::stateCallsEditEventHandler(shared_ptr<FsmState> state)
{
    clearSelection();
    state->getGraphicalRepresentation()->setSelected(true);
    resources->selectedState(state, true);
}

void FsmScene::stateCallsRenameEventHandler(shared_ptr<FsmState> state)
{
    clearSelection();
    state->getGraphicalRepresentation()->setSelected(true);
    resources->selectedState(state, true, true);
}

void FsmScene::treatMenu(QAction* action)
{
    if (action->text() == tr("Add state"))
    {
        shared_ptr<FsmState> logicState = machine->addState();

        addState(logicState, this->mousePos);
    }
    else if (action->text() == tr("Add initial state"))
    {
        shared_ptr<FsmState> logicState = machine->addState();
        logicState->setInitial();

        addState(logicState, this->mousePos);
    }
}

void FsmScene::transitionCallsEditEventHandler(shared_ptr<FsmTransition> transition)
{
    clearSelection();
    transition->getGraphicalRepresentation()->setSelected(true);
    resources->selectedTransition(transition, true);
}

FsmGraphicalState* FsmScene::addState(shared_ptr<FsmState> logicState, QPointF location)
{
    // Create graphical state
    FsmGraphicalState *newState = new FsmGraphicalState(logicState);
    addItem(newState);
    newState->setPos(location);
    newState->setZValue(1);

    QGraphicsItemGroup* actionsBox = newState->getActionsBox();
    addItem(actionsBox);
    actionsBox->setZValue(3);

    connect(newState, &FsmGraphicalState::editStateCalledEvent, this, &FsmScene::stateCallsEditEventHandler);
    connect(newState, &FsmGraphicalState::renameStateCalledEvent, this, &FsmScene::stateCallsRenameEventHandler);

    return newState;
}

void FsmScene::addTransition(FsmGraphicalTransition* newTransition)
{
    connect(newTransition, &FsmGraphicalTransition::dynamicSourceCalledEvent, this, &FsmScene::transitionCallsDynamicSourceEventHandler);
    connect(newTransition, &FsmGraphicalTransition::dynamicTargetCalledEvent, this, &FsmScene::transitionCallsDynamicTargetEventHandler);
    connect(newTransition, &FsmGraphicalTransition::editCalledEvent, this, &FsmScene::transitionCallsEditEventHandler);

    addItem(newTransition);
    newTransition->setZValue(2);

    QGraphicsTextItem* condition = newTransition->getConditionText();
    addItem(condition);
    condition->setZValue(3);

    QGraphicsItemGroup* actionsBox = newTransition->getActionsBox();
    addItem(actionsBox);
    actionsBox->setZValue(3);
}
