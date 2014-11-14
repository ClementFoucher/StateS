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
#include <QVBoxLayout>

#include <QDebug>

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
#include "contextmenu.h"

SimulatorTab::SimulatorTab(Fsm* machine, QWidget* parent) :
    QWidget(parent)
{
    this->machine = machine;

    this->setLayout(new QVBoxLayout());

    QLabel* title = new QLabel("<b>" + tr("Simulator") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(title);

    this->buttonTriggerSimulation = new QPushButton(tr("Start simulation"));
    this->buttonTriggerSimulation->setCheckable(true);
    connect(this->buttonTriggerSimulation, SIGNAL(clicked(bool)), this, SLOT(triggerSimulationMode(bool)));
    this->layout()->addWidget(this->buttonTriggerSimulation);
}

SimulatorTab::~SimulatorTab()
{
    // Close simulation
    this->triggerSimulationMode(false);
}

void SimulatorTab::triggerSimulationMode(bool enabled)
{
    if (enabled)
    {
        if (this->simulationTools == nullptr)
        {
            if (this->machine->getInitialState() != nullptr)
            {
                // First thing to do would be to check machine correctness

                this->buttonTriggerSimulation->setText(tr("End simulation"));

                this->clock = new Clock();
                connect(this->clock, SIGNAL(clockEvent()), this, SLOT(clockEvent()));
                connect(this->clock, SIGNAL(resetEvent()), this, SLOT(resetEvent()));

                this->simulationTools = new QWidget();
                this->simulationTools->setLayout(new QVBoxLayout());
                this->layout()->addWidget(this->simulationTools);

                QPushButton* buttonReset = new QPushButton(tr("Reset"));
                connect(buttonReset, SIGNAL(clicked()), this, SLOT(reset()));
                this->simulationTools->layout()->addWidget(buttonReset);

                QPushButton * buttonNextStep = new QPushButton(">");
                connect(buttonNextStep, SIGNAL(clicked()), this, SLOT(nextStep()));
                this->simulationTools->layout()->addWidget(buttonNextStep);

                QLabel* inputListTitle = new QLabel(tr("Select signals from the list below to activate inputs:"));
                inputListTitle->setAlignment(Qt::AlignCenter);
                this->simulationTools->layout()->addWidget(inputListTitle);

                this->inputList = new QListWidget();
                foreach (Input* io, this->machine->getInputs())
                {
                    this->inputList->addItem(io->getName());
                }
                this->inputList->setSelectionMode(QAbstractItemView::MultiSelection);
                connect(this->inputList, SIGNAL(itemSelectionChanged()), this, SLOT(updateInputs()));
                this->simulationTools->layout()->addWidget(this->inputList);

                this->reset();

                this->timeLine = new SimulationTimeLine(this->machine, this->clock, 0);
                this->timeLine->show();

                emit beginSimulation();
            }
            else
            {
                this->buttonTriggerSimulation->setChecked(false);

                ContextMenu* menu = ContextMenu::createErrorMenu(tr("No initial state!"));
                menu->popup(buttonTriggerSimulation->mapToGlobal(QPoint(buttonTriggerSimulation->width(), -menu->sizeHint().height())));
            }
        }
        else
        {
            qDebug() << "(Simulator tab:) Error! Trying to begin simulation while already launched!";
        }
    }
    else
    {
        if (this->simulationTools != nullptr)
        {
            delete this->inputList;
            this->inputList = nullptr;

            delete this->simulationTools;
            this->simulationTools = nullptr;

            delete this->timeLine;
            this->timeLine = nullptr;

            // Disconnects useless?
            disconnect(this->clock, SIGNAL(clockEvent()), this, SLOT(clockEvent()));
            disconnect(this->clock, SIGNAL(resetEvent()), this, SLOT(resetEvent()));
            delete this->clock;
            this->clock = nullptr;

            foreach(FsmState* state, this->machine->getStates())
            {
                state->setActive(false);
            }

            this->buttonTriggerSimulation->setText(tr("Start simulation"));

            emit endSimulation();
        }
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
