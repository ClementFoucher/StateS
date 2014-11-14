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
#include <QGridLayout>
#include <QIcon>
#include <QBitmap>
#include <QPixmap>
#include <fsmgraphicaltransition.h>

#include <QDebug>

#include "fsmtools.h"
#include "fsmgraphicalstate.h"
 #include "fsmstate.h"

FsmTools::FsmTools(QWidget* parent) :
    MachineTools(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    this->buttonNoTool = new QPushButton(tr("Mouse"));
//    this->buttonNoTool->setIcon(QIcon(*QCursor(Qt::CursorShape::ArrowCursor).bitmap()));
    this->buttonNoTool->setCheckable(true);
    this->buttonNoTool->setChecked(true);
    this->buttonNoTool->setEnabled(false);
    layout->addWidget(this->buttonNoTool, 0, 0, 1, 2);

    this->buttonAddInitialState = new QPushButton(tr("Add initial state"));
    this->buttonAddInitialState->setIcon(QIcon(FsmGraphicalState::getPixmap(128, true)));
    this->buttonAddInitialState->setCheckable(true);
    layout->addWidget(this->buttonAddInitialState, 1, 0, 1, 2);

    this->buttonAddState = new QPushButton(tr("Add state"));
    this->buttonAddState->setIcon(QIcon(FsmGraphicalState::getPixmap(128)));
    this->buttonAddState->setCheckable(true);
    layout->addWidget(this->buttonAddState, 2, 0, 1, 1);

    this->buttonAddTransition = new QPushButton(tr("Add transition"));
    this->buttonAddTransition->setIcon(QIcon(FsmGraphicalTransition::getPixmap(128)));
    this->buttonAddTransition->setCheckable(true);
    layout->addWidget(this->buttonAddTransition, 2, 1, 1, 1);

    this->signalMapper = new QSignalMapper(this);
    this->signalMapper->setMapping(buttonAddState, "add_state");
    this->signalMapper->setMapping(buttonAddInitialState, "add_initial_state");
    this->signalMapper->setMapping(buttonAddTransition, "add_transition");
    this->signalMapper->setMapping(buttonNoTool, "no_tool");

    connect(this->buttonAddState,        SIGNAL(clicked()), this->signalMapper, SLOT (map()));
    connect(this->buttonAddInitialState, SIGNAL(clicked()), this->signalMapper, SLOT (map()));
    connect(this->buttonAddTransition,   SIGNAL(clicked()), this->signalMapper, SLOT (map()));
    connect(this->buttonNoTool,          SIGNAL(clicked()), this->signalMapper, SLOT (map()));

    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(buttonPushed(QString)));
}

FsmTools::~FsmTools()
{
    delete this->signalMapper;
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
    if (this->buttonAddTransition->isChecked())
        return tool::transition;
    else if (this->buttonAddState->isChecked())
        return tool::state;
    else if (this->buttonAddInitialState->isChecked())
        return tool::initial_state;
    else
        return tool::none;
}

bool FsmTools::setTool(MachineTools::tool newTool)
{
    switch (newTool)
    {
    case MachineTools::tool::state:
        this->buttonNoTool->setChecked(false);
        this->buttonAddState->setChecked(true);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->setChecked(false);
        this->buttonNoTool->setEnabled(true);

        return true;

    case MachineTools::tool::transition:
        this->buttonNoTool->setChecked(false);
        this->buttonAddState->setChecked(false);
        this->buttonAddTransition->setChecked(true);
        this->buttonAddInitialState->setChecked(false);
        this->buttonNoTool->setEnabled(true);

        return true;

    case MachineTools::tool::initial_state:
        this->buttonNoTool->setChecked(false);
        this->buttonAddState->setChecked(false);
        this->buttonAddInitialState->setChecked(true);
        this->buttonAddTransition->setChecked(false);
        this->buttonNoTool->setEnabled(true);

        return true;

    case MachineTools::tool::none:
        this->buttonNoTool->setChecked(true);
        this->buttonAddState->setChecked(false);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->setChecked(false);
        this->buttonNoTool->setEnabled(false);

        return true;

    default:
        qDebug() << "(FSM Tool bar:) I can't use this tool!";
        return false;
    }
}
