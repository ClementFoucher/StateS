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
#include "scenewidget.h"


// Qt classes
#include <QResizeEvent>
#include <QStyle>
#include <QScrollBar>

// StateS classes
#include "fsmscene.h"
#include "fsm.h"
#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"
#include "machinetools.h"


SceneWidget::SceneWidget(shared_ptr<Machine> machine, ResourceBar* resources, QWidget* parent) :
    SceneWidget(parent)
{
    this->resourcesBar = resources;

    this->setMachine(machine, resources);
}

SceneWidget::SceneWidget(QWidget* parent) :
    QGraphicsView(parent)
{
    this->buttonZoomIn = new QPushButton("+", this);
    this->buttonZoomIn->resize(QSize(20, 20));
    connect(buttonZoomIn, &QAbstractButton::clicked, this, &SceneWidget::zoomIn);

    this->buttonZoomOut = new QPushButton("-", this);
    this->buttonZoomOut->resize(QSize(20, 20));
    connect(buttonZoomOut, &QAbstractButton::clicked, this, &SceneWidget::zoomOut);
}

void SceneWidget::setMachine(shared_ptr<Machine> machine, ResourceBar* resources)
{
    delete this->scene();

    this->resourcesBar = resources;

    if (machine != nullptr)
    {
        if (machine->getType() == Machine::type::FSM)
        {
            FsmScene* newScene = new FsmScene(dynamic_pointer_cast<Fsm>(machine), resources);
            newScene->setDisplaySize(this->size());

            this->setScene(newScene);
        }
    }
}

void SceneWidget::resizeEvent(QResizeEvent* event)
{
    if (scene() != nullptr)
        ((GenericScene*)scene())->setDisplaySize(event->size());

    int rightAlign = 10;
    if (this->verticalScrollBar()->isVisible())
        rightAlign += this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

    buttonZoomIn->move(this->width() - buttonZoomIn->width() - rightAlign, 10);
    buttonZoomOut->move(this->width() - buttonZoomOut->width() - rightAlign, buttonZoomIn->height() + 20);
}

void SceneWidget::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == Qt::LeftButton)
    {
        this->setDragMode(QGraphicsView::RubberBandDrag);
    }
    if (me->button() == Qt::MiddleButton)
    {
        this->movingScene = true;
        this->setDragMode(QGraphicsView::ScrollHandDrag);
    }
    else if (me->button() == Qt::RightButton)
    {
        if (this->resourcesBar->getBuildTools() != nullptr)
        {
            this->unsetCursor();
        }
    }

    QGraphicsView::mousePressEvent(me);
}

void SceneWidget::mouseReleaseEvent(QMouseEvent *me)
{
    if (this->dragMode() == QGraphicsView::ScrollHandDrag)
    {
        this->movingScene = false;
        this->setDragMode(QGraphicsView::NoDrag);
    }

    QGraphicsView::mouseReleaseEvent(me);
}

void SceneWidget::mouseMoveEvent(QMouseEvent *me)
{
    static QPoint lastMouseEventPos(0, 0);
    if (this->movingScene)
    {
        QScrollBar *hBar = horizontalScrollBar();
        QScrollBar *vBar = verticalScrollBar();
        QPoint delta = me->pos() - lastMouseEventPos;
        hBar->setValue(hBar->value() + -delta.x());
        vBar->setValue(vBar->value() - delta.y());
    }

    lastMouseEventPos = me->pos();

    QGraphicsView::mouseMoveEvent(me);
}

void SceneWidget::wheelEvent(QWheelEvent* event)
{
    if ( (event->modifiers() & Qt::ControlModifier) != 0)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

        double scaleFactor = 1.15;
        if(event->delta() > 0)
        {
            scale(scaleFactor, scaleFactor);
        }
        else
        {
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    }
    else if ( (event->modifiers() & Qt::ShiftModifier) != 0)
    {
        QScrollBar *hBar = horizontalScrollBar();
        hBar->setValue(hBar->value() - event->delta());
    }
    else
    {
        QScrollBar *vBar = verticalScrollBar();
        vBar->setValue(vBar->value() - event->delta());
    }
}

void SceneWidget::enterEvent(QEvent*)
{
    if (this->resourcesBar->getBuildTools() != nullptr)
    {
        if (this->resourcesBar->getBuildTools()->getTool() == MachineTools::tool::none)
        {
            this->unsetCursor();
        }
        else if (this->resourcesBar->getBuildTools()->getTool() == MachineTools::tool::state)
        {
            // Should test is FSM mode before
            QPixmap pixmap = FsmGraphicalState::getPixmap(32, false, true);
            this->setCursor(QCursor(pixmap, 0, 0));
        }
        else if (this->resourcesBar->getBuildTools()->getTool() == MachineTools::tool::initial_state)
        {
            // Should test is FSM mode before
            QPixmap pixmap = FsmGraphicalState::getPixmap(32, true, true);

            this->setCursor(QCursor(pixmap, 0, 0));
        }
        else if (this->resourcesBar->getBuildTools()->getTool() == MachineTools::tool::transition)
        {
            // Should test is FSM mode before
            QPixmap pixmap = FsmGraphicalTransition::getPixmap(32);
            QCursor cursor(pixmap, 0 , 0);

            this->setCursor(cursor);
        }
        else
            this->unsetCursor();
    }

}

void SceneWidget::zoomIn()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    scale(1.15, 1.15);
}

void SceneWidget::zoomOut()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    scale(1/1.15, 1/1.15);
}
