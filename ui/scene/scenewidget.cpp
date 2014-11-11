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

#include <QResizeEvent>
#include <QStyle>
#include <QScrollBar>
#include <fsmstate.h>

#include "scenewidget.h"

#include "machine.h"
#include "fsmscene.h"
#include "resourcesbar.h"
#include "fsm.h"
#include "fsmgraphicalstate.h"

SceneWidget::SceneWidget(Machine* machine, ResourcesBar* resources, QWidget* parent) :
    SceneWidget(parent)
{
    this->setMachine(machine, resources);
}

SceneWidget::SceneWidget(QWidget* parent) :
    QGraphicsView(parent)
{
    this->buttonZoomIn = new QPushButton("+", this);
    this->buttonZoomIn->resize(QSize(20, 20));
    connect(buttonZoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));

    this->buttonZoomOut = new QPushButton("-", this);
    this->buttonZoomOut->resize(QSize(20, 20));
    connect(buttonZoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
}

void SceneWidget::setMachine(Machine* machine, ResourcesBar* resources)
{
    delete this->scene();

    if (machine != nullptr)
    {
        if (machine->getType() == Machine::type::FSM)
        {
            FsmScene* newScene = new FsmScene(resources, (Fsm*)machine);
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
