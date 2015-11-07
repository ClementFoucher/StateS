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
#include "fsmgraphicstate.h"

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>

// StateS classes
#include "machine.h"
#include "fsmgraphictransition.h"
#include "fsmstate.h"
#include "fsmscene.h"
#include "signal.h"
#include "contextmenu.h"
#include "fsm.h"
#include "machinebuilder.h"


//
// Static elements
//

const qreal FsmGraphicState::radius = 50;
QBrush FsmGraphicState::inactiveBrush = QBrush(QColor(230,230,230), Qt::SolidPattern);
QBrush FsmGraphicState::activeBrush = QBrush(Qt::green, Qt::SolidPattern);
QPen FsmGraphicState::pen = QPen(Qt::SolidPattern, 3);

qreal FsmGraphicState::getRadius()
{
    return radius;
}

//
// Class object definition
//

QPixmap FsmGraphicState::getPixmap(uint size, bool isInitial, bool addArrow)
{
    QPixmap pixmap(QSize(size, size));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    painter.setPen(FsmGraphicState::pen);
    painter.setBrush(FsmGraphicState::inactiveBrush);
    painter.drawEllipse(QRectF(FsmGraphicState::pen.width()/2, FsmGraphicState::pen.width()/2, size-FsmGraphicState::pen.width(), size-FsmGraphicState::pen.width()));

    if (isInitial)
    {
        qreal space = size/10 + FsmGraphicState::pen.width();
        painter.drawEllipse(QRectF(FsmGraphicState::pen.width()/2 + space, FsmGraphicState::pen.width()/2 + space, size-FsmGraphicState::pen.width() - space*2, size-FsmGraphicState::pen.width() - space*2));
    }

    if (addArrow)
    {
        painter.drawLine(0, 0, size/3, 0);
        painter.drawLine(0, 0, 0, size/3);
    }

    return pixmap;
}

FsmGraphicState::FsmGraphicState(shared_ptr<FsmState> logicState) :
    QGraphicsEllipseItem(-radius, -radius, 2*radius, 2*radius)
{
    this->setPen(pen);

    this->setFlag(QGraphicsItem::ItemIsMovable);
    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsFocusable);
    this->setFlag(ItemSendsScenePositionChanges);

    this->actionsBox = new QGraphicsItemGroup();

    logicState->setGraphicRepresentation(this);
    connect(logicState.get(), &MachineComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicState::rebuildRepresentation);
    connect(logicState.get(), &MachineComponent::componentDynamicStateChangedEvent,        this, &FsmGraphicState::rebuildRepresentation);

    this->logicState = logicState;

    shared_ptr<Fsm> machine = logicState->getOwningFsm();
    connect(machine.get(), &Fsm::changedModeEvent, this, &FsmGraphicState::machineModeChangedEventHandler);

    rebuildRepresentation();
}

FsmGraphicState::~FsmGraphicState()
{
    delete actionsBox;

    shared_ptr<FsmState> l_logicState = this->logicState.lock();

    if (l_logicState != nullptr)
    {
        disconnect(l_logicState.get(), &MachineComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicState::rebuildRepresentation);
        disconnect(l_logicState.get(), &MachineComponent::componentDynamicStateChangedEvent,        this, &FsmGraphicState::rebuildRepresentation);

        shared_ptr<Fsm> machine = l_logicState->getOwningFsm();
        if (machine != nullptr)
            disconnect(machine.get(), &Fsm::changedModeEvent, this, &FsmGraphicState::machineModeChangedEventHandler);

        l_logicState->clearGraphicRepresentation();
    }
}

shared_ptr<FsmState> FsmGraphicState::getLogicState() const
{
    return this->logicState.lock();
}

QVariant FsmGraphicState::itemChange(GraphicsItemChange change, const QVariant& value)
{
    // Inform connected transitions we are moving
    if ((change == GraphicsItemChange::ItemPositionChange) || (change == GraphicsItemChange::ItemPositionHasChanged))
    {
        if (this->moveEventEnabled)
            emit stateMovingEvent();
    }
    else if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedChange)
    {
        // If changing to selected
        if (value.toBool() == true)
        {
            // Refuse selection if there is a transition already selected
            foreach(QGraphicsItem* selectedItem, this->scene()->selectedItems())
            {
                if (dynamic_cast<FsmGraphicTransition*>(selectedItem) != nullptr)
                    return (QVariant)false;
            }
        }
    }

    // Reposition action box
    if (scene() != nullptr)
        actionsBox->setPos(mapToScene(0,0));

    return QGraphicsEllipseItem::itemChange(change, value);
}


void FsmGraphicState::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    shared_ptr<FsmState> l_logicState = this->logicState.lock();

    if (l_logicState != nullptr)
    {
        ContextMenu* menu = new ContextMenu();
        menu->addTitle(tr("State") + " <i>" + l_logicState->getName() + "</i>");

        Machine::mode currentMode = l_logicState->getOwningFsm()->getCurrentMode();

        if (currentMode == Machine::mode::editMode )
        {
            if (!l_logicState->isInitial())
                menu->addAction(tr("Set initial"));
            menu->addAction(tr("Edit"));
            menu->addAction(tr("Draw transition from this state"));
            menu->addAction(tr("Rename"));
            menu->addAction(tr("Delete"));
        }
        else if (currentMode == Machine::mode::simulateMode )
        {
            if (! l_logicState->getIsActive())
            {
                menu->addAction(tr("Set active"));
            }
        }

        if (menu->actions().count() > 1) // > 1 because title is always here
        {
            menu->popup(event->screenPos());

            connect(menu, &QMenu::triggered, this, &FsmGraphicState::treatMenu);
        }
        else
        {
            delete menu;
        }
    }
}

void FsmGraphicState::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        askDelete();
    }
    else if (event->key() == Qt::Key_Menu)
    {
        QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

        QGraphicsView * view = scene()->views()[0];

        QPoint posOnParent = view->mapFromScene(this->scenePos());

        QPoint posOnScreen = view->mapToGlobal(posOnParent);
        contextEvent->setScreenPos(posOnScreen);

        this->contextMenuEvent(contextEvent);
    }
    else if ( (event->key() == Qt::Key_Right) |
              (event->key() == Qt::Key_Left) |
              (event->key() == Qt::Key_Up) |
              (event->key() == Qt::Key_Down)
            )
    {
        if ((event->modifiers() & Qt::ShiftModifier) != 0)
        {
            // Small moves
            if (event->key() == Qt::Key_Right)
                this->setPos(this->pos() + QPointF(1,0));
            else if (event->key() == Qt::Key_Left)
                this->setPos(this->pos() + QPointF(-1,0));
            else if (event->key() == Qt::Key_Up)
                this->setPos(this->pos() + QPointF(0, -1));
            else if (event->key() == Qt::Key_Down)
                this->setPos(this->pos() + QPointF(0, 1));
        }
        else
        {
            if (event->key() == Qt::Key_Right)
                this->setPos(this->pos() + QPointF(10,0));
            else if (event->key() == Qt::Key_Left)
                this->setPos(this->pos() + QPointF(-10,0));
            else if (event->key() == Qt::Key_Up)
                this->setPos(this->pos() + QPointF(0, -10));
            else if (event->key() == Qt::Key_Down)
                this->setPos(this->pos() + QPointF(0, 10));
        }
    }
    else if (event->key() == Qt::Key_F2)
    {
        emit renameStateCalledEvent(this->logicState.lock());
    }
    else
    {
        event->ignore();
    }
}

/**
 * @brief Move even is inhibited until the state
 * is actually placed on the scene. This function
 * enables it afterwhat.
 * @param enable
 */
void FsmGraphicState::enableMoveEvent()
{
    this->moveEventEnabled = true;
}

void FsmGraphicState::treatMenu(QAction* action)
{
    shared_ptr<FsmState> l_logicState = this->logicState.lock();

    if (l_logicState != nullptr)
    {
        if (action->text() == tr("Edit"))
        {
            emit editStateCalledEvent(l_logicState);
        }
        else if (action->text() == tr("Set initial"))
        {
                l_logicState->setInitial();
        }
        else if (action->text() == tr("Rename"))
        {
            emit renameStateCalledEvent(l_logicState);
        }
        else if (action->text() == tr("Delete"))
        {
            askDelete();
        }
        else if (action->text() == tr("Draw transition from this state"))
        {
            ((FsmScene*)scene())->beginDrawTransition(this, QPointF());
            l_logicState->getOwningFsm()->getMachineBuilder()->setSingleUseTool(MachineBuilder::singleUseTool::drawTransitionFromScene);
        }
        else if (action->text() == tr("Set active"))
        {
                l_logicState->getOwningFsm()->forceStateActivation(l_logicState);
        }
    }
}

void FsmGraphicState::machineModeChangedEventHandler(Machine::mode)
{
    this->rebuildRepresentation();
}

void FsmGraphicState::askDelete()
{
    shared_ptr<FsmState> l_logicState = this->logicState.lock();
    shared_ptr<Fsm> owningFsm = l_logicState->getOwningFsm();

    int linkedTransitions = l_logicState->getOutgoingTransitions().count() + l_logicState->getIncomingTransitions().count();
    if (linkedTransitions != 0)
    {
        QString messageText = tr("Delete current state?") + "<br />";
        if (linkedTransitions == 1)
            messageText += tr("The connected transition will be deleted");
        else
            messageText += tr("All") + " " + QString::number(linkedTransitions) + " " + tr("connected transitions will be deleted");

        QMessageBox::StandardButton reply = QMessageBox::question(scene()->views()[0], tr("User confirmation needed"), messageText, QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            // This call will destroy the current object as consequence of the logic object destruction
            owningFsm->removeState(l_logicState);
        }
    }
    else
        owningFsm->removeState(l_logicState);
}

QGraphicsItemGroup* FsmGraphicState::getActionsBox() const
{
    return actionsBox;
}

void FsmGraphicState::rebuildRepresentation()
{
    // Clear all child items
    qDeleteAll(this->childItems());
    this->childItems().clear();

    shared_ptr<FsmState> l_logicState = this->logicState.lock();

    if (l_logicState != nullptr)
    {
        if (l_logicState->getIsActive())
            this->setBrush(activeBrush);
        else
            this->setBrush(inactiveBrush);

        stateName = new QGraphicsTextItem(l_logicState->getName(), this);

        stateName->setPos(-stateName->boundingRect().width()/2, -stateName->boundingRect().height()/2);

        if (l_logicState->isInitial())
        {
            QGraphicsEllipseItem* insideCircle = new QGraphicsEllipseItem(QRect(-(radius-10), -(radius-10), 2*(radius-10), 2*(radius-10)), this);
            insideCircle->setPen(pen);
        }

        qDeleteAll(actionsBox->childItems());
        actionsBox->childItems().clear();

        QList<shared_ptr<Signal>> actions = l_logicState->getActions();
        if (actions.count() != 0)
        {
            qreal textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();
            qreal maxTextWidth = 0;

            for (int i = 0 ; i < actions.count() ; i++)
            {
                QGraphicsTextItem* actionText = new QGraphicsTextItem(actions[i]->getText(), actionsBox);

                QString currentActionText;

                Machine::mode currentMode = l_logicState->getOwningFsm()->getCurrentMode();

                if ( (scene() != nullptr) && (currentMode == Machine::mode::simulateMode) )
                    currentActionText = actions[i]->getText(true);
                else
                    currentActionText = actions[i]->getText(false);

                if (l_logicState->getActionType(actions[i]) == MachineActuatorComponent::action_types::set)
                    currentActionText += " = 1";
                else if (l_logicState->getActionType(actions[i]) == MachineActuatorComponent::action_types::reset)
                {
                    if (actions[i]->getSize() == 1)
                        currentActionText += " = 0";
                    else
                        currentActionText += " = " + LogicValue::getValue0(actions[i]->getSize()).toString() + "<sub>b</sub>";
                }
                else if (l_logicState->getActionType(actions[i]) == MachineActuatorComponent::action_types::assign)
                    currentActionText += " = " + l_logicState->getActionValue(actions[i]).toString() + "<sub>b</sub>";

                actionText->setHtml(currentActionText);


                if (maxTextWidth < actionText->boundingRect().width())
                    maxTextWidth = actionText->boundingRect().width();
                actionText->setPos(radius+20, -( ( (textHeight*actions.count()) / 2) ) + i*textHeight);
                actionText->setZValue(1);
            }

            QPainterPath actionBorderPath;
            actionBorderPath.lineTo(20,                0);
            actionBorderPath.lineTo(20,                ((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(20 + maxTextWidth, ((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(20 + maxTextWidth, -((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(20,                -((qreal)actions.count()/2)*textHeight);
            actionBorderPath.lineTo(20,                0);

            QGraphicsPathItem* stateActionsOutline = new QGraphicsPathItem(actionBorderPath, actionsBox);
            stateActionsOutline->setPen(pen);
            stateActionsOutline->setBrush(QBrush(Qt::white, Qt::Dense3Pattern));
            stateActionsOutline->setZValue(0);
            stateActionsOutline->setPos(radius, 0);

            actionsBox->setPos(mapToScene(0,0));// Positions must be expressed wrt. scene, ast this is not a child of this (scene stacking issues)
        }
    }
}
