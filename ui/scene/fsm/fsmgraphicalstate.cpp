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
#include "fsmgraphicalstate.h"

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QResizeEvent>
#include <QMessageBox>

// StateS classes
#include "fsmstate.h"
#include "fsmscene.h"
#include "signal.h"
#include "scenewidget.h"
#include "contextmenu.h"
#include "fsm.h"


//
// Static elements
//

const qreal FsmGraphicalState::radius = 50;
QBrush FsmGraphicalState::inactiveBrush = QBrush(QColor(230,230,230), Qt::SolidPattern);
QBrush FsmGraphicalState::activeBrush = QBrush(Qt::green, Qt::SolidPattern);
QPen FsmGraphicalState::pen = QPen(Qt::SolidPattern, 3);

qreal FsmGraphicalState::getRadius()
{
    return radius;
}

//
// Class object definition
//

QPixmap FsmGraphicalState::getPixmap(uint size, bool isInitial, bool addArrow)
{
    QPixmap pixmap(QSize(size, size));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    painter.setPen(FsmGraphicalState::pen);
    painter.setBrush(FsmGraphicalState::inactiveBrush);
    painter.drawEllipse(QRectF(FsmGraphicalState::pen.width()/2, FsmGraphicalState::pen.width()/2, size-FsmGraphicalState::pen.width(), size-FsmGraphicalState::pen.width()));

    if (isInitial)
    {
        qreal space = size/10 + FsmGraphicalState::pen.width();
        painter.drawEllipse(QRectF(FsmGraphicalState::pen.width()/2 + space, FsmGraphicalState::pen.width()/2 + space, size-FsmGraphicalState::pen.width() - space*2, size-FsmGraphicalState::pen.width() - space*2));
    }

    if (addArrow)
    {
        painter.drawLine(0, 0, size/3, 0);
        painter.drawLine(0, 0, 0, size/3);
    }

    return pixmap;
}

// This is only used to build state icon from image.
// Do not try to call rebuild representation on a
// graphical state created with this construcor!
FsmGraphicalState::FsmGraphicalState()
{
    this->setPen(pen);
}

FsmGraphicalState::FsmGraphicalState(shared_ptr<FsmState> logicState) :
    QGraphicsEllipseItem(-radius, -radius, 2*radius, 2*radius)
{
    this->setPen(pen);

    this->setFlag(QGraphicsItem::ItemIsMovable);
    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsFocusable);
    this->setFlag(ItemSendsScenePositionChanges);

    this->actionsBox = new QGraphicsItemGroup();

    logicState->setGraphicalRepresentation(this);
    connect(logicState.get(), &MachineActuatorComponent::componentStaticConfigurationChangedEvent, this, &FsmGraphicalState::rebuildRepresentation);
    connect(logicState.get(), &FsmState::componentDynamicStateChangedEvent,                        this, &FsmGraphicalState::rebuildRepresentation);

    this->logicalState = logicState;

    rebuildRepresentation();
}

FsmGraphicalState::~FsmGraphicalState()
{
    delete actionsBox;

    shared_ptr<FsmState> state = this->logicalState.lock();
    if (state != nullptr)
        state->clearGraphicalRepresentation();
}

shared_ptr<FsmState> FsmGraphicalState::getLogicalState() const
{
    return this->logicalState.lock();
}

QVariant FsmGraphicalState::itemChange(GraphicsItemChange change, const QVariant& value)
{
    // Inform connected transitions we are moving
    if ((change == GraphicsItemChange::ItemPositionChange) || (change == GraphicsItemChange::ItemPositionHasChanged))
        emit stateMovingEvent();

    // Reposition action box
    if (scene() != nullptr)
        actionsBox->setPos(mapToScene(0,0));

    return QGraphicsEllipseItem::itemChange(change, value);
}


void FsmGraphicalState::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    ContextMenu* menu = new ContextMenu();
    shared_ptr<FsmState> currentState = logicalState.lock();

    menu->addTitle(tr("State") + " <i>" + currentState->getName() + "</i>");

    if (((FsmScene*)this->scene())->getMode() == ResourceBar::mode::editMode )
    {
        if (!currentState->isInitial())
            menu->addAction(tr("Set initial"));
        menu->addAction(tr("Edit"));
        menu->addAction(tr("Draw transition from this state"));
        menu->addAction(tr("Rename"));
        menu->addAction(tr("Delete"));
    }
    else if (((FsmScene*)this->scene())->getMode() == ResourceBar::mode::simulateMode )
    {
        if (!this->logicalState.lock()->getIsActive())
        {
            menu->addAction(tr("Set active"));
        }
    }

    if (menu->actions().count() > 1) // > 1 because title is always here
    {
        menu->popup(event->screenPos());

        connect(menu, &QMenu::triggered, this, &FsmGraphicalState::treatMenu);
        connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    }
    else
    {
        delete menu;
    }
}

void FsmGraphicalState::keyPressEvent(QKeyEvent *event)
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
        emit renameStateCalledEvent(this->logicalState.lock());
    }
    else
    {
        event->ignore();
    }
}



void FsmGraphicalState::treatMenu(QAction* action)
{
    if (action->text() == tr("Edit"))
    {
        emit editStateCalledEvent(logicalState.lock());
    }
    else if (action->text() == tr("Set initial"))
    {
        logicalState.lock()->setInitial();
    }
    else if (action->text() == tr("Rename"))
    {
        emit renameStateCalledEvent(logicalState.lock());
    }
    else if (action->text() == tr("Delete"))
    {
        askDelete();
    }
    else if (action->text() == tr("Draw transition from this state"))
    {
        ((FsmScene*)scene())->beginDrawTransition(this, QPointF());
    }
    else if (action->text() == tr("Set active"))
    {
        this->logicalState.lock()->getOwningFsm()->forceStateActivation(this->logicalState.lock());
    }
}

void FsmGraphicalState::askDelete()
{
    shared_ptr<FsmState> state = this->logicalState.lock();
    shared_ptr<Fsm> owningFsm = state->getOwningFsm();

    int linkedTransitions = state->getOutgoingTransitions().count() + state->getIncomingTransitions().count();
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
            // This call will destroy the current object as consequence of the logical object destruction
            owningFsm->removeState(state);
        }
    }
    else
        owningFsm->removeState(state);
}

QGraphicsItemGroup* FsmGraphicalState::getActionsBox() const
{
    return actionsBox;
}

void FsmGraphicalState::rebuildRepresentation()
{
    // Clear all child items
    qDeleteAll(this->childItems());
    this->childItems().clear();

    shared_ptr<FsmState> state = this->logicalState.lock();

    if (state->getIsActive())
        this->setBrush(activeBrush);
    else
        this->setBrush(inactiveBrush);

    stateName = new QGraphicsTextItem(state->getName(), this);

    stateName->setPos(-stateName->boundingRect().width()/2, -stateName->boundingRect().height()/2);

    if (state->isInitial())
    {
        QGraphicsEllipseItem* insideCircle = new QGraphicsEllipseItem(QRect(-(radius-10), -(radius-10), 2*(radius-10), 2*(radius-10)), this);
        insideCircle->setPen(pen);
    }

    qDeleteAll(actionsBox->childItems());
    actionsBox->childItems().clear();

    QList<shared_ptr<Signal>> actions = state->getActions();
    if (actions.count() != 0)
    {
        qreal textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();
        qreal maxTextWidth = 0;

        for (int i = 0 ; i < actions.count() ; i++)
        {
            QGraphicsTextItem* actionText = new QGraphicsTextItem(actions[i]->getText(), actionsBox);

            QString currentActionText;

            if ( (scene() != nullptr) && (((FsmScene*)scene())->getMode() == ResourceBar::mode::simulateMode) )
                currentActionText = actions[i]->getText(true);
            else
                currentActionText = actions[i]->getText(false);

            if (state->getActionType(actions[i]) == MachineActuatorComponent::action_types::set)
                currentActionText += " = 1";
            else if (state->getActionType(actions[i]) == MachineActuatorComponent::action_types::reset)
            {
                if (actions[i]->getSize() == 1)
                    currentActionText += " = 0";
                else
                    currentActionText += " = " + LogicValue::getValue0(actions[i]->getSize()).toString() + "<sub>b</sub>";
            }
            else if (state->getActionType(actions[i]) == MachineActuatorComponent::action_types::assign)
                currentActionText += " = " + state->getActionValue(actions[i]).toString() + "<sub>b</sub>";

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
