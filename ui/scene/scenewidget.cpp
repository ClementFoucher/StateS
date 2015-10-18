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
#include <QPushButton>

// StateS classes
#include "fsmscene.h"
#include "fsm.h"
#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"


SceneWidget::SceneWidget(shared_ptr<Machine> machine, ResourceBar* resources, QWidget* parent) :
    SceneWidget(parent)
{
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

void SceneWidget::setMachine(shared_ptr<Machine> newMachine, ResourceBar* resources)
{
    // Clear
    delete this->scene();

    // Disconnect
    shared_ptr<MachineBuilder> oldMachineBuilder = this->machineBuilder.lock();
    if (oldMachineBuilder != nullptr)
        disconnect(oldMachineBuilder.get(), &MachineBuilder::changedToolEvent, this, &SceneWidget::toolChangedEventHandler);

    // Initialize
    if (newMachine != nullptr)
    {
        shared_ptr<MachineBuilder> newMachineBuiler = newMachine->getMachineBuilder();
        this->machineBuilder = newMachineBuiler;

        connect(newMachineBuiler.get(), &MachineBuilder::changedToolEvent, this, &SceneWidget::toolChangedEventHandler);

        if (newMachine->getType() == Machine::type::FSM)
        {
            FsmScene* newScene = new FsmScene(dynamic_pointer_cast<Fsm>(newMachine), resources);
            newScene->setDisplaySize(this->size());

            this->setScene(newScene);
        }
    }
}

void SceneWidget::toolChangedEventHandler(MachineBuilder::tool newTool)
{
    // TODO when add other machines types: will need to test machine type

    if      (newTool == MachineBuilder::tool::none)
    {
        this->unsetCursor();
    }
    else if (newTool == MachineBuilder::tool::state)
    {
        QPixmap pixmap = FsmGraphicalState::getPixmap(32, false, true);
        this->setCursor(QCursor(pixmap, 0, 0));
    }
    else if (newTool == MachineBuilder::tool::initial_state)
    {
        QPixmap pixmap = FsmGraphicalState::getPixmap(32, true, true);

        this->setCursor(QCursor(pixmap, 0, 0));
    }
    else if (newTool == MachineBuilder::tool::transition)
    {
        QPixmap pixmap = FsmGraphicalTransition::getPixmap(32);
        QCursor cursor(pixmap, 0 , 0);

        this->setCursor(cursor);
    }
    else
        this->unsetCursor();

}

void SceneWidget::resizeEvent(QResizeEvent* event)
{
    // Inform scene view has been modified

    if (scene() != nullptr)
        ((GenericScene*)scene())->setDisplaySize(event->size());

    // Relocate overlay buttons

    int rightAlign = 10;
    if (this->verticalScrollBar()->isVisible())
        rightAlign += this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

    buttonZoomIn->move(this->width() - buttonZoomIn->width() - rightAlign, 10);
    buttonZoomOut->move(this->width() - buttonZoomOut->width() - rightAlign, buttonZoomIn->height() + 20);

    // Transmit event

    QGraphicsView::resizeEvent(event);
}

void SceneWidget::mousePressEvent(QMouseEvent* me)
{
    // In this function, do not transmit event in case we handle it
    bool transmitEvent = true;

    shared_ptr<MachineBuilder> machineBuilder = this->machineBuilder.lock();

    if (me->button() == Qt::LeftButton)
    {
        if (machineBuilder != nullptr)
        {
            if (machineBuilder->getTool() == MachineBuilder::tool::none)
            {
                this->setDragMode(QGraphicsView::RubberBandDrag);
                // We changed drag mode, but now it is handled by some component in stack: need to transmit event
            }

        }
    }
    else if (me->button() == Qt::MiddleButton)
    {
        this->movingScene = true;
        this->setDragMode(QGraphicsView::ScrollHandDrag); // Just for mouse icon, not using its properties
        transmitEvent = false;
    }
    else if (me->button() == Qt::RightButton)
    {
        if (machineBuilder != nullptr)
        {
            if (machineBuilder->getTool() != MachineBuilder::tool::none)
            {
                machineBuilder->setTool(MachineBuilder::tool::none);
                transmitEvent = false;
            }
        }
    }

    if (transmitEvent)
        QGraphicsView::mousePressEvent(me);

}

void SceneWidget::mouseReleaseEvent(QMouseEvent* me)
{
    // In this function, do not transmit event in case we handle it
    bool transmitEvent = true;

    // In this function, drag mode is handled by some component in stack, so we need to transmit event

    if (this->dragMode() == QGraphicsView::ScrollHandDrag) // Or use movingScene?
    {
        this->movingScene = false;
        this->setDragMode(QGraphicsView::NoDrag);
        transmitEvent = false;
    }

    if (transmitEvent)
        QGraphicsView::mouseReleaseEvent(me);
}

void SceneWidget::mouseMoveEvent(QMouseEvent* me)
{
    static QPoint lastMouseEventPos(0, 0);

    // In this function, do not transmit event in case we handle it
    bool transmitEvent = true;

    if (this->movingScene)
    {
        QScrollBar* hBar = horizontalScrollBar();
        QScrollBar* vBar = verticalScrollBar();
        QPoint delta = me->pos() - lastMouseEventPos;
        hBar->setValue(hBar->value() + -delta.x());
        vBar->setValue(vBar->value() - delta.y());
        transmitEvent = false;
    }

    lastMouseEventPos = me->pos();

    if (transmitEvent)
        QGraphicsView::mouseMoveEvent(me);
}

void SceneWidget::wheelEvent(QWheelEvent* event)
{
    // In this function, do never transmit event: we always handle it

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
