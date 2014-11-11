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

#include <QLabel>

#include <QDebug>

#include "toolresourcestab.h"

#include "fsmtools.h"

ToolResourcesTab::ToolResourcesTab(Machine::type machineType, QWidget* parent) :
    QWidget(parent)
{
    if (machineType == Machine::type::FSM)
    {
        buildTools = new FsmTools();

        QLabel* title = new QLabel("<b>" + tr("FSM editor") + "</b>");
        title->setAlignment(Qt::AlignCenter);

        layout = new QVBoxLayout(this);
        layout->addWidget(title);
        layout->addWidget(buildTools);

        this->setMaximumHeight(250);
    }
    else
    {
        qDebug() << "(Drawing tool bar:) Error, unknown machine type";
    }

}

MachineTools* ToolResourcesTab::getBuildTools() const
{
    return buildTools;
}
