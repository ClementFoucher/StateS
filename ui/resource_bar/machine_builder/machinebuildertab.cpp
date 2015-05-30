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
#include "machinebuildertab.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsItem>

// Debug
#include <QDebug>

// StateS classes
#include "fsmtools.h"
#include "machine.h"


MachineBuilderTab::MachineBuilderTab(Machine::type machineType, shared_ptr<Machine> machine, QWidget* parent) :
    QWidget(parent)
{
    this->machine = machine;

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);

    QLabel* instrTitle = new QLabel("<b>" + tr("Navigation") + "</b>");
    instrTitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(instrTitle);

    QString instructions;

    instructions += tr("Use <i>mouse center button (wheel)</i> to move scene:");
    instructions += "<br />";
    instructions += tr("Press and hold wheel for free move,");
    instructions += "<br />";
    instructions += tr("Scroll wheel to move vertically,");
    instructions += "<br />";
    instructions += tr("Scroll wheel while holding <i>shift</i> to move horizontally.");
    instructions += "<br />";
    instructions += "<br />";
    instructions += tr("Use <i>ctrl+mouse wheel</i> to zoom in/out");
    instructions += "<br />";
    instructions += "<br />";
    instructions += tr("Use <i>right-click</i> to unselect current tool") + "<br />" + tr("or display context menu if no tool selected");
    instructions += "<br />";

    QLabel* instructionsLabel = new QLabel(instructions);
    instructionsLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(instructionsLabel);

    if (machineType == Machine::type::FSM)
    {
        QLabel* title = new QLabel("<b>" + tr("FSM editor") + "</b>");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        buildTools = new FsmTools();
        layout->addWidget(buildTools);
    }
    else
    {
        qDebug() << "(Drawing tool bar:) Error, unknown machine type";
    }

    QLabel* visuTitle = new QLabel("<b>" + tr("Machine visualization") + "</b>");
    visuTitle ->setAlignment(Qt::AlignCenter);
    layout->addWidget(visuTitle);

    componentVisualization = new QGraphicsView();
    componentVisualization->setScene(new QGraphicsScene());
    this->updateMachineVisualization();
    layout->addWidget(componentVisualization);

    connect(machine.get(), &Machine::inputListChangedEvent,  this, &MachineBuilderTab::updateMachineVisualization);
    connect(machine.get(), &Machine::outputListChangedEvent, this, &MachineBuilderTab::updateMachineVisualization);

    this->setLayout(layout);
}

MachineTools* MachineBuilderTab::getBuildTools() const
{
    return buildTools;
}

QGraphicsScene* MachineBuilderTab::getComponentVisualizationScene()
{
    return this->componentVisualization->scene();
}

void MachineBuilderTab::updateMachineVisualization()
{
    componentVisualization->scene()->clear();

    QGraphicsItem* component = machine.lock()->getComponentVisualization();
    componentVisualization->scene()->addItem(component);

    /*QRectF bound = componentVisualization->scene()->sceneRect();
    bound.adjust(0, -10, 0, 10);
    componentVisualization->scene()->setSceneRect(bound);*/
}
