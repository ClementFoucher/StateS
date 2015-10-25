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
    this->sceneMode = sceneMode_e::noScene;

    this->buttonZoomIn  = new QPushButton("+", this);
    this->buttonZoomOut = new QPushButton("-", this);

    this->buttonZoomIn ->resize(QSize(20, 20));
    this->buttonZoomOut->resize(QSize(20, 20));

    connect(buttonZoomIn,  &QAbstractButton::clicked, this, &SceneWidget::zoomIn);
    connect(buttonZoomOut, &QAbstractButton::clicked, this, &SceneWidget::zoomOut);

    this->updateDragMode();
}

void SceneWidget::setMachine(shared_ptr<Machine> newMachine, ResourceBar* resources)
{
    // Clear
    delete this->scene();

    // Disconnect
    shared_ptr<MachineBuilder> oldMachineBuilder = this->machineBuilder.lock();
    if (oldMachineBuilder != nullptr)
    {
        disconnect(oldMachineBuilder.get(), &MachineBuilder::changedToolEvent, this, &SceneWidget::toolChangedEventHandler);
        disconnect(oldMachineBuilder.get(), &MachineBuilder::singleUseToolSelected, this, &SceneWidget::singleUseToolChangedEventHandler);
    }

    // Initialize
    if (newMachine != nullptr)
    {
        shared_ptr<MachineBuilder> newMachineBuiler = newMachine->getMachineBuilder();
        this->machineBuilder = newMachineBuiler;

        connect(newMachineBuiler.get(), &MachineBuilder::changedToolEvent, this, &SceneWidget::toolChangedEventHandler);
        connect(newMachineBuiler.get(), &MachineBuilder::singleUseToolSelected, this, &SceneWidget::singleUseToolChangedEventHandler);

        shared_ptr<Fsm> newFsm = dynamic_pointer_cast<Fsm>(newMachine);

        if (newFsm != nullptr)
        {
            FsmScene* newScene = new FsmScene(newFsm, resources);
            newScene->setDisplaySize(this->size());

            this->setScene(newScene);
        }

        this->sceneMode = sceneMode_e::idle;
    }
    else
    {
        this->sceneMode = sceneMode_e::noScene;
    }

    this->updateDragMode();
}

void SceneWidget::toolChangedEventHandler(MachineBuilder::tool newTool)
{
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

    this->updateDragMode();
}

void SceneWidget::singleUseToolChangedEventHandler(MachineBuilder::singleUseTool newTool)
{
    if ( (newTool == MachineBuilder::singleUseTool::drawTransitionFromScene) ||
         (newTool == MachineBuilder::singleUseTool::editTransitionSource) ||
         (newTool == MachineBuilder::singleUseTool::editTransitionTarget)
         )
    {
        QPixmap pixmap = FsmGraphicalTransition::getPixmap(32);
        QCursor cursor(pixmap, 0 , 0);

        this->setCursor(cursor);
        this->updateDragMode();
    }
    else
    {
        shared_ptr<MachineBuilder> machineBuilder = this->machineBuilder.lock();
        if (machineBuilder != nullptr)
        {
            this->toolChangedEventHandler(machineBuilder->getTool());
        }
    }
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

    if (me->button() == Qt::MiddleButton)
    {
        this->sceneMode = sceneMode_e::movingScene;
        this->setDragMode(QGraphicsView::ScrollHandDrag); // Just for mouse icon, not using its properties
        transmitEvent = false;
    }

    if (transmitEvent)
        QGraphicsView::mousePressEvent(me);

}

void SceneWidget::mouseMoveEvent(QMouseEvent* me)
{
    static QPoint lastMouseEventPos(0, 0);

    // In this function, do not transmit event in case we handle it
    bool transmitEvent = true;

    if (this->sceneMode == sceneMode_e::movingScene)
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

void SceneWidget::mouseReleaseEvent(QMouseEvent* me)
{
    // In this function, do not transmit event in case we handle it
    bool transmitEvent = true;

    if (this->sceneMode == sceneMode_e::movingScene)
    {
        this->sceneMode = sceneMode_e::idle;
        this->updateDragMode();

        transmitEvent = false;
    }

    if (transmitEvent)
        QGraphicsView::mouseReleaseEvent(me);
}

void SceneWidget::mouseDoubleClickEvent(QMouseEvent* me)
{
    bool transmitEvent = true;

    if (this->sceneMode == sceneMode_e::movingScene)
    {
        transmitEvent = false;
    }

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

void SceneWidget::updateDragMode()
{
    QGraphicsView::DragMode dragMode = QGraphicsView::NoDrag;

    if (this->sceneMode == sceneMode_e::idle)
    {
        shared_ptr<MachineBuilder> machineBuilder = this->machineBuilder.lock();

        if (machineBuilder != nullptr)
        {
            if (machineBuilder->getTool() == MachineBuilder::tool::none)
            {
                dragMode = QGraphicsView::RubberBandDrag;
            }
        }

    }

    this->setDragMode(dragMode);
}
