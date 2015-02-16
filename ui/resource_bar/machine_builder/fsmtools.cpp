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

// Current class header
#include "fsmtools.h"

// Qt classes
#include <QGridLayout>

// Debug
#include <QDebug>

// StateS classes
#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"


FsmTools::FsmTools(QWidget* parent) :
    MachineTools(parent)
{
    this->buttonNoTool          = new QPushButton(tr("Mouse"));
    this->buttonAddInitialState = new QPushButton(tr("Add initial state"));
    this->buttonAddState        = new QPushButton(tr("Add state"));
    this->buttonAddTransition   = new QPushButton(tr("Add transition"));

//    this->buttonNoTool->setIcon(QIcon(*QCursor(Qt::CursorShape::ArrowCursor).bitmap()));
    this->buttonAddInitialState->setIcon(QIcon(FsmGraphicalState::getPixmap(128, true)));
    this->buttonAddState->       setIcon(QIcon(FsmGraphicalState::getPixmap(128)));
    this->buttonAddTransition->  setIcon(QIcon(FsmGraphicalTransition::getPixmap(128)));

    this->buttonNoTool->         setCheckable(true);
    this->buttonAddInitialState->setCheckable(true);
    this->buttonAddState->       setCheckable(true);
    this->buttonAddTransition->  setCheckable(true);

    this->buttonNoTool->setChecked(true);
    this->buttonNoTool->setEnabled(false);

    connect(this->buttonNoTool,          &QAbstractButton::clicked, this, &FsmTools::buttonNotToolPushed);
    connect(this->buttonAddInitialState, &QAbstractButton::clicked, this, &FsmTools::buttonAddInitialStatePushed);
    connect(this->buttonAddState,        &QAbstractButton::clicked, this, &FsmTools::buttonAddStatePushed);
    connect(this->buttonAddTransition,   &QAbstractButton::clicked, this, &FsmTools::buttonAddTransitionPushed);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(this->buttonNoTool,          0, 0, 1, 2);
    layout->addWidget(this->buttonAddInitialState, 1, 0, 1, 2);
    layout->addWidget(this->buttonAddState,        2, 0, 1, 1);
    layout->addWidget(this->buttonAddTransition,   2, 1, 1, 1);
}

void FsmTools::buttonAddStatePushed(bool activated)
{
    if (activated)
        setTool(MachineTools::tool::state);
    else
        setTool(MachineTools::tool::none);
}

void FsmTools::buttonAddInitialStatePushed(bool activated)
{
    if (activated)
        setTool(MachineTools::tool::initial_state);
    else
        setTool(MachineTools::tool::none);
}

void FsmTools::buttonAddTransitionPushed(bool activated)
{
    if (activated)
        setTool(MachineTools::tool::transition);
    else
        setTool(MachineTools::tool::none);
}

void FsmTools::buttonNotToolPushed(bool)
{
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
        this->buttonNoTool->         setChecked(false);
        this->buttonAddState->       setChecked(true);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->  setChecked(false);
        this->buttonNoTool->         setEnabled(true);

        return true;

    case MachineTools::tool::transition:
        this->buttonNoTool->         setChecked(false);
        this->buttonAddState->       setChecked(false);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->  setChecked(true);
        this->buttonNoTool->         setEnabled(true);

        return true;

    case MachineTools::tool::initial_state:
        this->buttonNoTool->         setChecked(false);
        this->buttonAddState->       setChecked(false);
        this->buttonAddInitialState->setChecked(true);
        this->buttonAddTransition->  setChecked(false);
        this->buttonNoTool->         setEnabled(true);

        return true;

    case MachineTools::tool::none:
        this->buttonNoTool->         setChecked(true);
        this->buttonAddState->       setChecked(false);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->  setChecked(false);
        this->buttonNoTool->         setEnabled(false);

        return true;

    default:
        qDebug() << "(FSM Tool bar:) I can't use this tool!";
        return false;
    }
}
