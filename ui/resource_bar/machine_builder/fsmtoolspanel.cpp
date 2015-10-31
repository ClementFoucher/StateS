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
#include "fsmtoolspanel.h"

// Qt classes
#include <QGridLayout>
#include <QPushButton>

// Debug
#include <QDebug>

// StateS classes
#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"


FsmToolsPanel::FsmToolsPanel(shared_ptr<MachineBuilder> machineBuilder, QWidget* parent) :
    MachineToolsPanel(machineBuilder, parent)
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

    connect(this->buttonNoTool,          &QAbstractButton::clicked, this, &FsmToolsPanel::buttonNotToolPushed);
    connect(this->buttonAddInitialState, &QAbstractButton::clicked, this, &FsmToolsPanel::buttonAddInitialStatePushed);
    connect(this->buttonAddState,        &QAbstractButton::clicked, this, &FsmToolsPanel::buttonAddStatePushed);
    connect(this->buttonAddTransition,   &QAbstractButton::clicked, this, &FsmToolsPanel::buttonAddTransitionPushed);

    connect(machineBuilder.get(), &MachineBuilder::changedToolEvent, this, &FsmToolsPanel::toolChangedEventHandler);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(this->buttonNoTool,          0, 0, 1, 2);
    layout->addWidget(this->buttonAddInitialState, 1, 0, 1, 2);
    layout->addWidget(this->buttonAddState,        2, 0, 1, 1);
    layout->addWidget(this->buttonAddTransition,   2, 1, 1, 1);
}

void FsmToolsPanel::buttonAddStatePushed(bool activated)
{
    shared_ptr<MachineBuilder> machineBuilder = this->machineBuilder.lock();

    if (machineBuilder != nullptr)
    {
        if (activated)
            machineBuilder->setTool(MachineBuilder::tool::state);
        else
            machineBuilder->setTool(MachineBuilder::tool::none);
    }
    else
        this->lockOnError();
}

void FsmToolsPanel::buttonAddInitialStatePushed(bool activated)
{
    shared_ptr<MachineBuilder> machineBuilder = this->machineBuilder.lock();

    if (machineBuilder != nullptr)
    {
        if (activated)
            machineBuilder->setTool(MachineBuilder::tool::initial_state);
        else
            machineBuilder->setTool(MachineBuilder::tool::none);
    }
    else
        this->lockOnError();
}

void FsmToolsPanel::buttonAddTransitionPushed(bool activated)
{
    shared_ptr<MachineBuilder> machineBuilder = this->machineBuilder.lock();

    if (machineBuilder != nullptr)
    {
        if (activated)
            machineBuilder->setTool(MachineBuilder::tool::transition);
        else
            machineBuilder->setTool(MachineBuilder::tool::none);
    }
    else
        this->lockOnError();
}

void FsmToolsPanel::buttonNotToolPushed(bool)
{
    shared_ptr<MachineBuilder> machineBuilder = this->machineBuilder.lock();

    if (machineBuilder != nullptr)
    {
        machineBuilder->setTool(MachineBuilder::tool::none);
    }
    else
        this->lockOnError();
}

void FsmToolsPanel::lockOnError()
{
    this->buttonNoTool->         setChecked(true);
    this->buttonAddState->       setChecked(false);
    this->buttonAddInitialState->setChecked(false);
    this->buttonAddTransition->  setChecked(false);

    this->buttonNoTool->         setEnabled(false);
    this->buttonAddState->       setEnabled(false);
    this->buttonAddInitialState->setEnabled(false);
    this->buttonAddTransition->  setEnabled(false);
}

bool FsmToolsPanel::toolChangedEventHandler(MachineBuilder::tool newTool)
{
    bool result = false;

    switch (newTool)
    {
    case MachineBuilder::tool::state:
        this->buttonNoTool->         setChecked(false);
        this->buttonAddState->       setChecked(true);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->  setChecked(false);
        this->buttonNoTool->         setEnabled(true);

        result = true;
        break;

    case MachineBuilder::tool::transition:
        this->buttonNoTool->         setChecked(false);
        this->buttonAddState->       setChecked(false);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->  setChecked(true);
        this->buttonNoTool->         setEnabled(true);

        result = true;
        break;

    case MachineBuilder::tool::initial_state:
        this->buttonNoTool->         setChecked(false);
        this->buttonAddState->       setChecked(false);
        this->buttonAddInitialState->setChecked(true);
        this->buttonAddTransition->  setChecked(false);
        this->buttonNoTool->         setEnabled(true);

        result = true;
        break;

    case MachineBuilder::tool::none:
    case MachineBuilder::tool::quittingTool:
        this->buttonNoTool->         setChecked(true);
        this->buttonAddState->       setChecked(false);
        this->buttonAddInitialState->setChecked(false);
        this->buttonAddTransition->  setChecked(false);
        this->buttonNoTool->         setEnabled(false);

        result =  true;
        break;

    default:
        qDebug() << "(FSM Tools panel:) I can't use this tool!";
    }

    return result;
}
