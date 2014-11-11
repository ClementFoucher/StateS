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

#include "interfaceresourcestab.h"

#include "variableeditortools.h"

InterfaceResourcesTab::InterfaceResourcesTab(Machine* machine, QWidget* parent) :
    QWidget(parent)
{
    QLabel* title = new QLabel("<b>" + tr("Machine I/O editor")  + "</b>");
    title->setAlignment(Qt::AlignCenter);

    QLabel* inputsTitle = new QLabel(tr("Inputs"));
    inputToolBox = new VariableEditorTools(machine, VariableEditorTools::type::inputs);

    QLabel* outputsTitle = new QLabel(tr("Outputs"));
    outputToolBox = new VariableEditorTools(machine, VariableEditorTools::type::outputs);

    QLabel* varTitle = new QLabel("<b>" + tr("Variable editor") + "</b>");
    varTitle->setAlignment(Qt::AlignCenter);
    variablesToolBox = new VariableEditorTools(machine, VariableEditorTools::type::variables);

    layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(inputsTitle);
    layout->addWidget(inputToolBox);
    layout->addWidget(outputsTitle);
    layout->addWidget(outputToolBox);
    layout->addWidget(varTitle);
    layout->addWidget(variablesToolBox);
}

