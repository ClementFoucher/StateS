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

#include "centralwidget.h"

#include <QResizeEvent>

#include "scenewidget.h"
#include "resourcesbar.h"
#include "genericscene.h"

CentralWidget::CentralWidget(Machine* machine, QWidget* parent) :
    QWidget(parent)
{
    this->verticalSplitterPos = 450;

    // Deletion useless, as we take ownership of QObjects
    this->machineDisplayArea = new SceneWidget(this);
    this->resourcesBar = new ResourcesBar(this);
    connect(this->resourcesBar, SIGNAL(simulationToggled()), this, SLOT(handleSimulationToggled()));

    this->setMachine(machine);
}

void CentralWidget::setMachine(Machine* machine)
{
    this->resourcesBar->setMachine(machine);
    this->machineDisplayArea->setMachine(machine, resourcesBar);
}

void CentralWidget::resizeEvent(QResizeEvent* event)
{
    this->machineDisplayArea->setGeometry(QRect(0, 0, event->size().width()-verticalSplitterPos-5, event->size().height()));
    this->resourcesBar->setGeometry(QRect(event->size().width()-verticalSplitterPos+5, 0, verticalSplitterPos-5, event->size().height()));
}

SceneWidget* CentralWidget::getMachineDisplayArea() const
{
    return this->machineDisplayArea;
}

void CentralWidget::handleSimulationToggled()
{
    ((GenericScene*)(machineDisplayArea->scene()))->simulationModeChanged();
}
