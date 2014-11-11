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

#include "fsmtools.h"

FsmTools::FsmTools(QWidget* parent) :
    MachineTools(parent)
{
    layout = new QGridLayout(this);

    buttonNoTool = new QPushButton(tr("Mouse"));
    buttonNoTool->setCheckable(true);
    buttonNoTool->setChecked(true);
    buttonNoTool->setEnabled(false);
    layout->addWidget(buttonNoTool, 0, 0, 1, 2);

    buttonAddInitialState = new QPushButton(tr("Add initial state"));
    buttonAddInitialState->setCheckable(true);
    layout->addWidget(buttonAddInitialState, 1, 0, 1, 2);

    buttonAddState = new QPushButton(tr("Add state"));
    buttonAddState->setCheckable(true);
    layout->addWidget(buttonAddState, 2, 0, 1, 1);

    buttonAddTransition = new QPushButton(tr("Add transition"));
    buttonAddTransition->setCheckable(true);
    layout->addWidget(buttonAddTransition, 2, 1, 1, 1);

    signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(buttonAddState, "add_state");
    signalMapper->setMapping(buttonAddInitialState, "add_initial_state");
    signalMapper->setMapping(buttonAddTransition, "add_transition");
    signalMapper->setMapping(buttonNoTool, "no_tool");

    connect(buttonAddState, SIGNAL(clicked()), signalMapper, SLOT (map()));
    connect(buttonAddInitialState, SIGNAL(clicked()), signalMapper, SLOT (map()));
    connect(buttonAddTransition, SIGNAL(clicked()), signalMapper, SLOT (map()));
    connect(buttonNoTool, SIGNAL(clicked()), signalMapper, SLOT (map()));

    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(buttonPushed(QString)));
}

void FsmTools::buttonPushed(QString button)
{
    if (button == "add_state")
        setTool(MachineTools::tool::state);
    else if (button == "add_transition")
        setTool(MachineTools::tool::transition);
    else if (button == "add_initial_state")
        setTool(MachineTools::tool::initial_state);
    else
        setTool(MachineTools::tool::none);
}

FsmTools::tool FsmTools::getTool() const
{
    if (buttonAddTransition->isChecked())
        return tool::transition;
    else if (buttonAddState->isChecked())
        return tool::state;
    else if (buttonAddInitialState->isChecked())
        return tool::initial_state;
    else
        return tool::none;
}

bool FsmTools::setTool(MachineTools::tool newTool)
{
    switch (newTool)
    {
    case MachineTools::tool::state:
        buttonNoTool->setChecked(false);
        buttonAddState->setChecked(true);
        buttonAddInitialState->setChecked(false);
        buttonAddTransition->setChecked(false);
        buttonNoTool->setEnabled(true);

        return true;

    case MachineTools::tool::transition:
        buttonNoTool->setChecked(false);
        buttonAddState->setChecked(false);
        buttonAddTransition->setChecked(true);
        buttonAddInitialState->setChecked(false);
        buttonNoTool->setEnabled(true);

        return true;

    case MachineTools::tool::initial_state:
        buttonNoTool->setChecked(false);
        buttonAddState->setChecked(false);
        buttonAddInitialState->setChecked(true);
        buttonAddTransition->setChecked(false);
        buttonNoTool->setEnabled(true);

        return true;

    case MachineTools::tool::none:
        buttonNoTool->setChecked(true);
        buttonAddState->setChecked(false);
        buttonAddInitialState->setChecked(false);
        buttonAddTransition->setChecked(false);


        buttonNoTool->setEnabled(false);
        return true;

    default:
        qDebug() << "(FSM Tool bar:) I can't use this tool!";
        return false;
    }
}
