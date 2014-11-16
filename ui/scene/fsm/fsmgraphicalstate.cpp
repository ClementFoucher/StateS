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

#include "fsmgraphicalstate.h"

#include <QPen>
#include <QGraphicsSceneContextMenuEvent>
#include <QAction>
#include <QLabel>
#include <QWidgetAction>
#include <QResizeEvent>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>

#include <QDebug>

#include "fsmstate.h"
#include "fsmscene.h"
#include "logicvariable.h"
#include "scenewidget.h"
#include "contextmenu.h"

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

FsmGraphicalState::FsmGraphicalState(FsmState* logicState) :
    QGraphicsEllipseItem(-radius, -radius, 2*radius, 2*radius)
{
    this->setPen(pen);

    this->setFlag(QGraphicsItem::ItemIsMovable);
    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsFocusable);
    this->setFlag(ItemSendsScenePositionChanges);

    this->actionsBox = new QGraphicsItemGroup();

    this->logicalState = logicState;
    this->logicalState->setGraphicalRepresentation(this);
    connect(this->logicalState, SIGNAL(stateConfigurationChanged()), this, SLOT(rebuildRepresentation()));

    rebuildRepresentation();
}

FsmGraphicalState::~FsmGraphicalState()
{
    delete actionsBox;

    disconnect(this->logicalState, SIGNAL(stateConfigurationChanged()), this, SLOT(rebuildRepresentation()));
    logicalState->clearGraphicalRepresentation();
}

FsmState* FsmGraphicalState::getLogicalState() const
{
    return this->logicalState;
}

QVariant FsmGraphicalState::itemChange(GraphicsItemChange change, const QVariant& value)
{
    // Inform connected transitions we are moving
    if ((change == GraphicsItemChange::ItemPositionChange) || (change == GraphicsItemChange::ItemPositionHasChanged))
        emit moving();

    // Reposition action box
    if (scene() != nullptr)
        actionsBox->setPos(mapToScene(0,0));

    return QGraphicsEllipseItem::itemChange(change, value);
}


void FsmGraphicalState::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    ContextMenu* menu = new ContextMenu();
    menu->addTitle(tr("State") + " <i>" + logicalState->getName() + "</i>");

    if (!logicalState->isInitial())
        menu->addAction(tr("Set initial"));
    menu->addAction(tr("Edit"));
    menu->addAction(tr("Draw transition from this state"));
    menu->addAction(tr("Rename"));
    menu->addAction(tr("Delete"));

    menu->popup(event->screenPos());

    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(treatMenu(QAction*)));
}

void FsmGraphicalState::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
        askDelete();
    else if (event->key() == Qt::Key_Menu)
    {
        QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

        QGraphicsView * view = scene()->views()[0];

        QPoint posOnParent = view->mapFromScene(this->scenePos());

        QPoint posOnScreen = view->mapToGlobal(posOnParent);
        contextEvent->setScreenPos(posOnScreen);

        this->contextMenuEvent(contextEvent);
    }
    else if ((event->modifiers() & Qt::ShiftModifier) != 0)
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

void FsmGraphicalState::treatMenu(QAction* action)
{
    if (action->text() == tr("Edit"))
    {
        emit callEdit(logicalState);
    }
    else if (action->text() == tr("Set initial"))
    {
        logicalState->setInitial();
    }
    else if (action->text() == tr("Rename"))
    {
        emit callRename(logicalState);
    }
    else if (action->text() == tr("Delete"))
    {
        askDelete();
    }
    else if (action->text() == tr("Draw transition from this state"))
    {
        ((FsmScene*)scene())->beginDrawTransition(this, QPointF());
    }
}

void FsmGraphicalState::askDelete()
{
    int linkedTransitions = this->logicalState->getOutgoingTransitions().count() + this->logicalState->getIncomingTransitions().count();
    if (linkedTransitions != 0)
    {
        QString messageText = tr("Delete current state?") + "<br />";
        if (linkedTransitions == 1)
            messageText += tr("The connected transition will be deleted");
        else
            messageText += tr("All ") + QString::number(linkedTransitions) + tr(" connected transitions will be deleted");

        QMessageBox::StandardButton reply = QMessageBox::question(scene()->views()[0], tr("User confirmation needed"), messageText, QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            // This call will destroy the current object as consequence of the logical object destruction
            delete logicalState;
        }
    }
    else
        delete logicalState;
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

    if (logicalState->getIsActive())
        this->setBrush(activeBrush);
    else
        this->setBrush(inactiveBrush);

    stateName = new QGraphicsTextItem(logicalState->getName(), this);

    stateName->setPos(-stateName->boundingRect().width()/2, -stateName->boundingRect().height()/2);

    if (logicalState->isInitial())
    {
        QGraphicsEllipseItem* insideCircle = new QGraphicsEllipseItem(QRect(-(radius-10), -(radius-10), 2*(radius-10), 2*(radius-10)), this);
        insideCircle->setPen(pen);
    }

    qDeleteAll(actionsBox->childItems());
    actionsBox->childItems().clear();

    QList<LogicVariable *> actions = logicalState->getActions();
    if (actions.count() != 0)
    {
        qreal textHeight = QGraphicsTextItem("Hello, world!").boundingRect().height();
        qreal maxTextWidth = 0;

        for (int i = 0 ; i < actions.count() ; i++)
        {
            QGraphicsTextItem* actionText = new QGraphicsTextItem(actions[i]->getText(), actionsBox);

            if (scene() != nullptr)
            {
                if (((FsmScene*)scene())->getMode() == ResourcesBar::mode::simulateMode)
                {
                    actionText->setHtml(actions[i]->getText(true));
                }
            }

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
