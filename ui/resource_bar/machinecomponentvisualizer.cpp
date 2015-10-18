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
#include "machinecomponentvisualizer.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QGraphicsItem>

// StateS classes
#include "machine.h"


MachineComponentVisualizer::MachineComponentVisualizer(shared_ptr<Machine> machine)
{
    this->machine = machine;

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);

    QLabel* visuTitle = new QLabel("<b>" + tr("Machine visualization") + "</b>");
    visuTitle ->setAlignment(Qt::AlignCenter);
    layout->addWidget(visuTitle);

    this->scene = shared_ptr<QGraphicsScene>(new QGraphicsScene());

    this->view = new QGraphicsView();
    this->view->setDragMode(QGraphicsView::ScrollHandDrag);
    this->view->setScene(this->scene.get());
    layout->addWidget(this->view);

    this->setLayout(layout);

    this->updateMachineVisualization();

    connect(machine.get(), &Machine::componentVisualizationUpdatedEvent,  this, &MachineComponentVisualizer::updateMachineVisualization);
}

shared_ptr<QGraphicsScene> MachineComponentVisualizer::getComponentVisualizationScene()
{
    return this->scene;
}

void MachineComponentVisualizer::wheelEvent(QWheelEvent* event)
{
    this->view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    double scaleFactor = 1.15;
    if(event->delta() > 0)
    {
        this->view->scale(scaleFactor, scaleFactor);
    }
    else
    {
        this->view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

void MachineComponentVisualizer::updateMachineVisualization()
{
    this->scene->clear();

    shared_ptr<Machine> machine = this->machine.lock();
    if (machine != nullptr)
    {
        QGraphicsItem* component = machine->getComponentVisualization();
        this->scene->addItem(component);
    }
}
