/*
 * Copyright © 2014-2017 Clément Foucher
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
#include <QVBoxLayout>
#include <QTabWidget>

// StateS classes
#include "signallisteditor.h"
#include "machinecomponentvisualizer.h"
#include "collapsiblewidgetwithtitle.h"


SignalEditorTab::SignalEditorTab(shared_ptr<Machine> machine, shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent) :
    QWidget(parent)
{
    this->machineComponentView = machineComponentView;

    QVBoxLayout* layout = new QVBoxLayout(this);

    //
    // Title

    QLabel* title = new QLabel("<b>" + tr("Signal editor")  + "</b>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    //
    // Signals

    QTabWidget* signalsTabs = new QTabWidget();
    layout->addWidget(signalsTabs);

    signalsTabs->insertTab(0, new SignalListEditor(machine, Machine::signal_type::Input),         tr("Inputs"));
    signalsTabs->insertTab(1, new SignalListEditor(machine, Machine::signal_type::Output),        tr("Outputs"));
    signalsTabs->insertTab(2, new SignalListEditor(machine, Machine::signal_type::LocalVariable), tr("Variables"));
    signalsTabs->insertTab(3, new SignalListEditor(machine, Machine::signal_type::Constant),      tr("Constants"));

    signalsTabs->setCurrentIndex(0);

    //
    // Hint

    QLabel* hintText = new QLabel(tr("Switch between signals types using tabs.")
                                  + "<br />"
                                  + tr("Signals length can not exceed 64 bits.")
                                  + "<br />"
                                  + tr("Allowed characters are alphanumerical ones, space and")
                                  + " {'_', '@', '#', '-'}."
                                  + "<br />"
                                  + tr("Double-click on a value to edit it.")
                                  );
    hintText->setAlignment(Qt::AlignCenter);
    hintText->setWordWrap(true);

    QString hintTitle = tr("Hint:") + " " + tr("Signal editor");
    this->hintDisplay = new CollapsibleWidgetWithTitle(hintTitle, hintText);
    layout->addWidget(this->hintDisplay);

    //
    // Machine visualization

    this->machineDisplay = new CollapsibleWidgetWithTitle(tr("Component visualization"), machineComponentView.get());
    layout->addWidget(this->machineDisplay);
}

void SignalEditorTab::setHintCollapsed(bool collapse)
{
    this->hintDisplay->setCollapsed(collapse);
}

void SignalEditorTab::setVisuCollapsed(bool collapse)
{
    this->machineDisplay->setCollapsed(collapse);
}

bool SignalEditorTab::getHintCollapsed()
{
    return this->hintDisplay->getCollapsed();
}

bool SignalEditorTab::getVisuCollapsed()
{
    return this->machineDisplay->getCollapsed();
}

void SignalEditorTab::showEvent(QShowEvent* e)
{
    // Ensure we get the view back
    shared_ptr<MachineComponentVisualizer> l_machineComponentView = this->machineComponentView.lock();

    if (l_machineComponentView != nullptr)
    {
        this->machineDisplay->setContent(tr("Component visualization"), l_machineComponentView.get());
    }

    QWidget::showEvent(e);
}
