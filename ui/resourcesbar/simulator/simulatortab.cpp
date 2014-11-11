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
#include <QListWidgetItem>
#include <QMenu>

#include "simulatortab.h"

#include "fsmstate.h"
#include "machine.h"
#include "fsm.h"
#include "fsmtransition.h"
#include "logicvariable.h"
#include "io.h"
#include "clock.h"
#include "simulationtimeline.h"
#include "logicequation.h"
#include "input.h"
#include "output.h"

SimulatorTab::SimulatorTab(Fsm* machine, QWidget* parent) :
    QWidget(parent)
{
    this->machine = machine;

    clock = new Clock();
    connect(clock, SIGNAL(clockEvent()), this, SLOT(clockEvent()));
    connect(clock, SIGNAL(resetEvent()), this, SLOT(resetEvent()));

    QLabel* title = new QLabel("<b>" + tr("Simulator") + "</b>");
    title->setAlignment(Qt::AlignCenter);

    buttonTriggerSimulation = new QPushButton(tr("Start simulation"));
    buttonTriggerSimulation->setCheckable(true);
    connect(buttonTriggerSimulation, SIGNAL(clicked(bool)), this, SLOT(triggerSimulationMode(bool)));

    layout = new QGridLayout(this);

    layout->addWidget(title, 0, 1, 1, 1);
    layout->addWidget(buttonTriggerSimulation, 1, 1, 1, 1);
}

SimulatorTab::~SimulatorTab()
{
    // Close simulation
    triggerSimulationMode(false);

    disconnect(clock, SIGNAL(clockEvent()), this, SLOT(clockEvent()));
    disconnect(clock, SIGNAL(resetEvent()), this, SLOT(resetEvent()));
    delete clock;
}

void SimulatorTab::triggerSimulationMode(bool enabled)
{
    if (enabled)
    {
        // First thing to do would be to check machine correctness

        if (machine->getInitialState() != nullptr)
        {

            buttonReset = new QPushButton(tr("Reset"));
            layout->addWidget(buttonReset, 2, 1, 1, 1);
            connect(buttonReset, SIGNAL(clicked()), this, SLOT(reset()));

            buttonNextStep = new QPushButton(">");
            connect(buttonNextStep, SIGNAL(clicked()), this, SLOT(nextStep()));
            layout->addWidget(buttonNextStep, 3, 1, 1, 1);

            inputList = new QListWidget();
            foreach (Input* io, machine->getInputs())
            {
                inputList->addItem(io->getName());
            }
            inputList->setSelectionMode(QAbstractItemView::MultiSelection);
            connect(inputList, SIGNAL(itemSelectionChanged()), this, SLOT(updateInputs()));
            layout->addWidget(inputList, 4, 1, 1, 1);

            reset();

            timeLine = new SimulationTimeLine(machine, clock, 0);
            timeLine->show();

            emit beginSimulation();
        }
        else
        {
            buttonTriggerSimulation->setChecked(false);

            QMenu* menu = new QMenu();
            menu->addAction(tr("No initial state!"));

            menu[0].setStyleSheet( QString( "background-color: lightgrey; border: 3px solid red; color: red"));

            menu->popup(buttonTriggerSimulation->mapToGlobal(QPoint(buttonTriggerSimulation->width(), -menu->sizeHint().height())));
        }
    }
    else
    {
        delete buttonReset;
        buttonReset = nullptr;

        delete buttonNextStep;
        buttonNextStep = nullptr;

        delete inputList;
        inputList = nullptr;

        delete timeLine;
        timeLine = nullptr;

        foreach(FsmState* state, machine->getStates())
        {
            state->setActive(false);
        }

        emit endSimulation();
    }
}


void SimulatorTab::nextStep()
{
    clock->nextStep();
}

void SimulatorTab::reset()
{
    clock->reset();
}

void SimulatorTab::resetEvent()
{
    if (machine->getInitialState() != nullptr)
    {
        foreach(FsmState* state, machine->getStates())
        {
            state->setActive(false);
        }

        currentState = machine->getInitialState();
        currentState->setActive(true);
    }
}

void SimulatorTab::clockEvent()
{
    foreach(LogicVariable* var, machine->getAllVariables())
    {
        var->updateState();
    }

    foreach(FsmTransition* transition, currentState->getOutgoingTransitions())
    {
        if (transition->getCondition()->isActive())
        {
            currentState->setActive(false);

            currentState = transition->getTarget();

            currentState->setActive(true);

            transition->setCrossed();

            break;
        }
    }
}


void SimulatorTab::updateInputs()
{
    foreach(LogicVariable* var, machine->getInputs())
    {
        foreach(QListWidgetItem* name, inputList->findItems("*", Qt::MatchWildcard))
        {
            if (name->text() == var->getName())
                var->setCurrentState(name->isSelected());
        }
    }
}
