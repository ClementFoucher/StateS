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
#include "signaleditortab.h"

// Qt classes
#include <QLabel>
#include <QToolBox>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QTabWidget>

// StateS classes
#include "signallisteditor.h"


SignalEditorTab::SignalEditorTab(shared_ptr<Machine> machine, QWidget* parent) :
    QWidget(parent)
{
    this->machine = machine;

    this->setLayout(new QVBoxLayout());

    //
    // Title

    QLabel* title = new QLabel("<b>" + tr("Signal editor")  + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(title);

    //
    // Help line

    QLabel* hint = new QLabel(tr("Switch between signals types using tabs below.")
                              + "<br />"
                              + tr("Signals length can not exceed 64 bits.")
                              + "<br />"
                              + tr("Allowed characters are alphanumerical ones, space and")
                              + " {'_', '@', '#', '-'}."
                              + "<br />"
                              + tr("Double-click on a value to edit it.")
                              );
    hint->setAlignment(Qt::AlignCenter);
    hint->setWordWrap(true);
    this->layout()->addWidget(hint);

    //
    // Signals

    QTabWidget* signalsTabs = new QTabWidget();
    this->layout()->addWidget(signalsTabs);

    signalsTabs->insertTab(0, new SignalListEditor(machine, Machine::signal_type::Input), tr("Inputs"));
    signalsTabs->insertTab(1, new SignalListEditor(machine, Machine::signal_type::Output), tr("Outputs"));
    signalsTabs->insertTab(2, new SignalListEditor(machine, Machine::signal_type::LocalVariable), tr("Variables"));
    signalsTabs->insertTab(3, new SignalListEditor(machine, Machine::signal_type::Constant), tr("Constants"));

    signalsTabs->setCurrentIndex(0);

    //
    // Machine visualization
    QLabel* visuTitle = new QLabel("<b>" + tr("Machine visualization")  + "</b>");
    visuTitle->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(visuTitle);

    componentVisualization = new QGraphicsView();
    componentVisualization->setScene(new QGraphicsScene());
    this->updateMachineVisualization();
    this->layout()->addWidget(componentVisualization);
    connect(machine.get(), &Machine::componentVisualizationUpdatedEvent,  this, &SignalEditorTab::updateMachineVisualization);
}

void SignalEditorTab::showEvent(QShowEvent*)
{
    this->updateMachineVisualization();
}

void SignalEditorTab::updateMachineVisualization()
{
    if (this->isVisible())
    {
        componentVisualization->scene()->clear();

        shared_ptr<QGraphicsItem> component = machine.lock()->getComponentVisualization();
        componentVisualization->scene()->addItem(component.get());
    }
}
