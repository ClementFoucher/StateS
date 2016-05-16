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
#include <QKeyEvent>
#include <QGraphicsView>

// Debug
#include <QDebug>

// StateS classes
#include "fsmgraphicstate.h"
#include "fsmgraphictransition.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "contextmenu.h"
#include "resourcebar.h"
#include "machinebuilder.h"


FsmScene::FsmScene(shared_ptr<Fsm> machine) :
    GenericScene()
{
    this->machine   = machine;
    this->sceneMode = sceneMode_e::idle;

    connect(this, &QGraphicsScene::selectionChanged, this, &FsmScene::handleSelection);
    connect(machine.get(), &Fsm::changedModeEvent, this, &FsmScene::simulationModeChanged);

    foreach(shared_ptr<FsmState> state, machine->getStates())
    {
        addState(state, state->position);
    }

    foreach(shared_ptr<FsmTransition> transition, machine->getTransitions())
    {
        addTransition(new FsmGraphicTransition(transition));
    }
}

FsmScene::~FsmScene()
{
    // We have to delete transitions first, so handle
    // item deletion manually

    QList<FsmGraphicTransition*> transitions;
    QList<FsmGraphicState*> states;

    foreach(QGraphicsItem* item, items())
    {
        FsmGraphicTransition* currentTransition = dynamic_cast<FsmGraphicTransition*>(item);

        if (currentTransition != nullptr)
            transitions.append(currentTransition);
    }

    foreach(QGraphicsItem* item, items())
    {
        FsmGraphicState* currentState = dynamic_cast<FsmGraphicState*>(item);

        if (currentState!= nullptr)
            states.append(currentState);
    }

    qDeleteAll(transitions);
    qDeleteAll(states);
}

void FsmScene::beginDrawTransition(FsmGraphicState* source, const QPointF& currentMousePos)
{
    this->sceneMode = sceneMode_e::drawingTransition;


    if (!currentMousePos.isNull())
        this->currentTransition = new FsmGraphicTransition(source, currentMousePos);
    else
    {
        // Compute mouse pos wrt. scene
        QPointF sceneMousePos = views()[0]->mapToScene(this->views()[0]->mapFromGlobal(QCursor::pos()));
        this->currentTransition = new FsmGraphicTransition(source, sceneMousePos);
    }

    addTransition(this->currentTransition);
}

void FsmScene::mousePressEvent(QGraphicsSceneMouseEvent* me)
{
    bool transmitEvent = true;
    shared_ptr<Fsm> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        shared_ptr<MachineBuilder> machineBuilder = l_machine->getMachineBuilder();

        if (machineBuilder != nullptr)
        {
            if (me->button() == Qt::LeftButton)
            {
                if (this->sceneMode == sceneMode_e::idle)
                {
                    MachineBuilder::tool currentTool = machineBuilder->getTool();

                    if (currentTool == MachineBuilder::tool::state)
                    {
                        // Create logic state
                        shared_ptr<FsmState> logicState = l_machine->addState();

                        // Create graphic state
                        addState(logicState, me->scenePos());

                        // Transmitting event so that new state is selected
                        // and can be moved within the same click
                    }
                    else if (currentTool == MachineBuilder::tool::initial_state)
                    {
                        // Create logic state
                        shared_ptr<FsmState> logicState = l_machine->addState();

                        // Create graphic state
                        addState(logicState, me->scenePos());

                        // Set state initial
                        logicState->setInitial();

                        // Transmitting event so that new state is selected
                        // and can be moved within the same click
                    }
                    else if (currentTool == MachineBuilder::tool::transition)
                    {
                        FsmGraphicState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

                        if ( stateUnderMouse != nullptr)
                        {
                            beginDrawTransition(stateUnderMouse, me->scenePos());
                            machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::beginDrawTransitionFromTool);
                            transmitEvent = false;
                        }
                    }
                    else if (currentTool == MachineBuilder::tool::none)
                    {
                        // If no tool, this is selection mode.
                        // Just do nothing and transmit event.
                    }
                    else
                    {
                        qDebug() << "(FsmScene:) Unsupported tool!";
                    }
                }
                else if ( (this->sceneMode == sceneMode_e::editingTransitionSource) || (this->sceneMode == sceneMode_e::editingTransitionTarget) )
                {
                    FsmGraphicState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

                    // TODO: Does left click on nothing should cancel insertion?
                    if (stateUnderMouse != nullptr)
                        this->currentTransition->endDynamicMode(true);
                    else
                        this->currentTransition->endDynamicMode(false);

                    this->sceneMode = sceneMode_e::goingBackToIdle;
                    machineBuilder->setTool(MachineBuilder::tool::quittingTool);
                    this->currentTransition = nullptr;

                    transmitEvent = false;
                }
            }
            else if (me->button() == Qt::RightButton)
            {
                if ( (this->sceneMode == sceneMode_e::editingTransitionSource) || (this->sceneMode == sceneMode_e::editingTransitionTarget) )
                {
                    // Cancel transition edition
                    this->sceneMode = sceneMode_e::goingBackToIdle;
                    machineBuilder->setTool(MachineBuilder::tool::quittingTool);
                    this->currentTransition->endDynamicMode(false);
                    this->currentTransition = nullptr;

                    transmitEvent = false;
                }
                else if (this->sceneMode == sceneMode_e::drawingTransition)
                {
                    // Cancel transition insertion
                    this->sceneMode = sceneMode_e::goingBackToIdle;
                    machineBuilder->setTool(MachineBuilder::tool::quittingTool);
                    delete this->currentTransition;
                    this->currentTransition = nullptr;

                    transmitEvent = false;
                }
                else if (machineBuilder->getTool() != MachineBuilder::tool::none)
                {
                    this->sceneMode = sceneMode_e::goingBackToIdle;
                    machineBuilder->setTool(MachineBuilder::tool::quittingTool);

                    transmitEvent = false;
                }
                // Else display a context menu, except if there is an item under mouse:
                // this is a context menu event on that particular item, just transmit event
                else if (this->items(me->scenePos(), Qt::IntersectsItemShape, Qt::DescendingOrder).count() == 0)
                {
                    // Display context menu
                    if (this->sceneMode != sceneMode_e::simulating) // Adding states is not allowed in simulation mode
                    {
                        ContextMenu* menu = new ContextMenu();
                        menu->addAction(tr("Add state"));
                        menu->addAction(tr("Add initial state"));
                        menu->popup(me->screenPos());

                        this->mousePos = me->scenePos();

                        connect(menu, &QMenu::triggered,   this, &FsmScene::treatMenu);
                        connect(menu, &QMenu::aboutToHide, this, &FsmScene::menuHiding);

                        this->sceneMode = sceneMode_e::askedForMenu;

                        transmitEvent = false;
                    }
                }
            }
        }

        if (transmitEvent)
            GenericScene::mousePressEvent(me);
    }
}

void FsmScene::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
    bool transmitEvent = true;
    static FsmGraphicState* previousStatePointed = nullptr;

    if ( (this->sceneMode == sceneMode_e::drawingTransition) ||
         (this->sceneMode == sceneMode_e::editingTransitionSource) ||
         (this->sceneMode == sceneMode_e::editingTransitionTarget)
       )
    {
        FsmGraphicState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

        if (stateUnderMouse != nullptr)
        {
            if ( (this->sceneMode == sceneMode_e::drawingTransition) || (this->sceneMode == sceneMode_e::editingTransitionTarget) )
            {
                if (stateUnderMouse != previousStatePointed)
                {
                    this->currentTransition->setTargetState(stateUnderMouse);
                    previousStatePointed = stateUnderMouse;
                }
            }
            else
            {

                if (stateUnderMouse != previousStatePointed)
                {
                    this->currentTransition->setSourceState(stateUnderMouse);
                    previousStatePointed = stateUnderMouse;
                }
            }
        }
        else
        {
            this->currentTransition->setMousePosition(me->scenePos());
            previousStatePointed = nullptr;
        }

        transmitEvent = false;
    }
    else if ( this->sceneMode == sceneMode_e::goingBackToIdle )
    {
        transmitEvent = false;
    }

    if (transmitEvent)
        GenericScene::mouseMoveEvent(me);
}

void FsmScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* me)
{
    bool transmitEvent = true;

    shared_ptr<Fsm> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        shared_ptr<MachineBuilder> machineBuilder = l_machine->getMachineBuilder();

        if (this->sceneMode == sceneMode_e::drawingTransition)
        {
            FsmGraphicState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

            if (stateUnderMouse != nullptr)
            {
                this->currentTransition->setTargetState(stateUnderMouse);
                this->currentTransition->endDynamicMode(true);

                shared_ptr<FsmState> source = this->currentTransition->getSource()->getLogicState();
                shared_ptr<FsmState> target = this->currentTransition->getTarget()->getLogicState();

                shared_ptr<FsmTransition> logicTransition(new FsmTransition(l_machine, source, target, nullptr, this->currentTransition));
                source->addOutgoingTransition(logicTransition);
                target->addIncomingTransition(logicTransition);
                this->currentTransition->setLogicTransition(logicTransition);
            }
            else
                delete this->currentTransition;

            this->currentTransition = nullptr;
            this->sceneMode = sceneMode_e::idle;
            machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::none);
            transmitEvent = false;
        }
        else if ( this->sceneMode == sceneMode_e::goingBackToIdle )
        {
            this->sceneMode = sceneMode_e::idle;

            if (machineBuilder->getTool() == MachineBuilder::tool::quittingTool)
                machineBuilder->setTool(MachineBuilder::tool::none);

            transmitEvent = false;
        }

        if (transmitEvent)
            GenericScene::mouseReleaseEvent(me);

        // TODO: better place for this call?
        updateSceneRect();
    }
}

void FsmScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* me)
{
    bool transmitEvent = true;

    if (this->sceneMode == sceneMode_e::drawingTransition)
        transmitEvent = false;
    else if (this->sceneMode == sceneMode_e::goingBackToIdle)
        transmitEvent = false;

    if (transmitEvent)
        GenericScene::mouseReleaseEvent(me);
}

void FsmScene::keyPressEvent(QKeyEvent* event)
{
    shared_ptr<Fsm> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        if (event->key() == Qt::Key_Escape)
        {
            shared_ptr<MachineBuilder> machineBuilder = l_machine->getMachineBuilder();

            if (this->sceneMode == sceneMode_e::drawingTransition)
            {
                this->sceneMode = sceneMode_e::idle;
                machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::none);
                delete this->currentTransition;
                this->currentTransition = nullptr;
            }
            else if ( (this->sceneMode == sceneMode_e::editingTransitionSource) || (this->sceneMode == sceneMode_e::editingTransitionTarget) )
            {
                // Cancel transition edition
                this->sceneMode = sceneMode_e::idle;
                machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::none);
                this->currentTransition->endDynamicMode(false);
                this->currentTransition = nullptr;
            }
            else
            {
                this->clearSelection();
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
                    FsmGraphicState* state = dynamic_cast<FsmGraphicState*>(item);

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
}

void FsmScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Do not show menus while a transition in a dynamic mode
    if ( (this->sceneMode == sceneMode_e::idle) || (this->sceneMode == sceneMode_e::simulating) )
        GenericScene::contextMenuEvent(event);
}

void FsmScene::transitionCallsDynamicSourceEventHandler(FsmGraphicTransition* transition)
{
    shared_ptr<Fsm> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        this->sceneMode = sceneMode_e::editingTransitionSource;
        l_machine->getMachineBuilder()->setSingleUseTool(MachineBuilder::singleUseTool::editTransitionSource);
        this->currentTransition = transition;

        clearSelection();
        transition->setSelected(true);

        this->currentTransition->setDynamicSourceMode(QPointF());
    }
}

void FsmScene::transitionCallsDynamicTargetEventHandler(FsmGraphicTransition* transition)
{
    shared_ptr<Fsm> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        this->sceneMode = sceneMode_e::editingTransitionTarget;
        l_machine->getMachineBuilder()->setSingleUseTool(MachineBuilder::singleUseTool::editTransitionTarget);
        this->currentTransition = transition;

        clearSelection();
        transition->setSelected(true);

        this->currentTransition->setDynamicTargetMode(QPointF());
    }
}

void FsmScene::handleSelection()
{
    // Set focus for context event
    if (this->selectedItems().count() == 1)
        this->selectedItems()[0]->setFocus();
    else
        this->clearFocus();

    // Updates resource panel selected item
    if (this->selectedItems().count() == 1)
    {
        FsmGraphicState* currentState = dynamic_cast< FsmGraphicState* >(this->selectedItems()[0]);
        if (currentState != nullptr)
        {
            emit itemSelectedEvent(currentState->getLogicState());
        }
        else
        {
            FsmGraphicTransition* currentTransition = dynamic_cast< FsmGraphicTransition* >(this->selectedItems()[0]);
            if (currentTransition!= nullptr)
            {
                emit itemSelectedEvent(currentTransition->getLogicTransition());
            }
        }
    }
    else
    {
        emit itemSelectedEvent(nullptr);
    }
}

FsmGraphicState* FsmScene::getStateAt(const QPointF& location) const
{
    QList<QGraphicsItem *>  itemsAtThisPoint = this->items(location, Qt::IntersectsItemShape, Qt::DescendingOrder); //this->((SceneWidget*)parent())->transform());
    // Warning: if using transform on view, the upper line should be adapted!


    foreach( QGraphicsItem* item, itemsAtThisPoint)
    {
        // Select the topmost visible state
        FsmGraphicState* currentItem = dynamic_cast<FsmGraphicState*> (item);

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

void FsmScene::simulationModeChanged(Machine::mode newMode)
{
    shared_ptr<Fsm> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        if (newMode == Machine::mode::editMode)
        {
            handleSelection();
            this->sceneMode = sceneMode_e::idle;
            l_machine->getMachineBuilder()->setSingleUseTool(MachineBuilder::singleUseTool::none);
        }
        else if (newMode == Machine::mode::simulateMode)
        {
            this->sceneMode = sceneMode_e::simulating;
            l_machine->getMachineBuilder()->setSingleUseTool(MachineBuilder::singleUseTool::none);
        }
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
    QRect baseDisplayRectangle(QPoint(-this->displaySize.width()/2, -this->displaySize.height()/2), this->displaySize);


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
    state->getGraphicRepresentation()->setSelected(true);
    emit editSelectedItemEvent();
}

void FsmScene::stateCallsRenameEventHandler(shared_ptr<FsmState> state)
{
    clearSelection();
    state->getGraphicRepresentation()->setSelected(true);
    emit renameSelectedItemEvent();
}

void FsmScene::treatMenu(QAction* action)
{
    shared_ptr<Fsm> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        this->sceneMode = sceneMode_e::idle;
        if (action->text() == tr("Add state"))
        {
            shared_ptr<FsmState> logicState = l_machine->addState();

            addState(logicState, this->mousePos);
        }
        else if (action->text() == tr("Add initial state"))
        {
            shared_ptr<FsmState> logicState = l_machine->addState();
            logicState->setInitial();

            addState(logicState, this->mousePos);
        }
    }
}

void FsmScene::menuHiding()
{
    this->sceneMode = sceneMode_e::idle;
}

void FsmScene::transitionCallsEditEventHandler(shared_ptr<FsmTransition> transition)
{
    clearSelection();
    transition->getGraphicRepresentation()->setSelected(true);
    emit editSelectedItemEvent();
}

FsmGraphicState* FsmScene::addState(shared_ptr<FsmState> logicState, QPointF location)
{
    // Create graphic state
    FsmGraphicState* newState = new FsmGraphicState(logicState);
    addItem(newState);
    newState->setPos(location);
    newState->setZValue(1);

    QGraphicsItemGroup* actionsBox = newState->getActionsBox();
    addItem(actionsBox);
    actionsBox->setZValue(3);

    connect(newState, &FsmGraphicState::editStateCalledEvent,   this, &FsmScene::stateCallsEditEventHandler);
    connect(newState, &FsmGraphicState::renameStateCalledEvent, this, &FsmScene::stateCallsRenameEventHandler);

    newState->enableMoveEvent();

    return newState;
}

void FsmScene::addTransition(FsmGraphicTransition* newTransition)
{
    connect(newTransition, &FsmGraphicTransition::dynamicSourceCalledEvent, this, &FsmScene::transitionCallsDynamicSourceEventHandler);
    connect(newTransition, &FsmGraphicTransition::dynamicTargetCalledEvent, this, &FsmScene::transitionCallsDynamicTargetEventHandler);
    connect(newTransition, &FsmGraphicTransition::editCalledEvent, this, &FsmScene::transitionCallsEditEventHandler);

    addItem(newTransition);
    newTransition->setZValue(2);

    QGraphicsTextItem* condition = newTransition->getConditionText();
    addItem(condition);
    condition->setZValue(3);

    QGraphicsItemGroup* actionsBox = newTransition->getActionsBox();
    addItem(actionsBox);
    actionsBox->setZValue(3);
}
