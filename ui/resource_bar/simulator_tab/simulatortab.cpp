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
#include "simulatortab.h"

// Qt classes
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QLineEdit>

// Debug
#include <QDebug>

// StateS classes
#include "fsmstate.h"
#include "fsm.h"
#include "fsmtransition.h"
#include "signal.h"
#include "clock.h"
#include "simulationwindow.h"
#include "contextmenu.h"
#include "inputsselector.h"


SimulatorTab::SimulatorTab(Fsm* machine, QWidget* parent) :
    QWidget(parent)
{
    this->machine = machine;

    this->setLayout(new QVBoxLayout());
    this->layout()->setAlignment(Qt::AlignTop);

    QLabel* title = new QLabel("<b>" + tr("Simulator") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(title);

    this->buttonTriggerSimulation = new QPushButton(tr("Start simulation"));
    this->buttonTriggerSimulation->setCheckable(true);
    connect(this->buttonTriggerSimulation, &QAbstractButton::clicked, this, &SimulatorTab::triggerSimulationMode);
    this->layout()->addWidget(this->buttonTriggerSimulation);
}

SimulatorTab::~SimulatorTab()
{
    // Close simulation
    this->triggerSimulationMode(false);
}

SimulationWindow *SimulatorTab::getTimeline() const
{
    return this->timeLine;
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
                connect(this->clock, &Clock::clockEvent, this, &SimulatorTab::clockEvent);
                connect(this->clock, &Clock::resetEvent, this, &SimulatorTab::resetEvent);

                this->simulationTools = new QWidget();
                this->simulationTools->setLayout(new QVBoxLayout());
                this->layout()->addWidget(this->simulationTools);

                QPushButton* buttonReset    = new QPushButton(tr("Reset"));
                QPushButton* buttonNextStep = new QPushButton("> " + tr("Do one step") + " >");
                this->buttonTriggerView     = new QPushButton(tr("View timeline"));

                QHBoxLayout* autoStepLayout = new QHBoxLayout();
                QLabel* autoStepBeginText = new QLabel(">> " + tr("Do one step every"));
                this->autoStepValue = new QLineEdit("1");
                QLabel* autoStepUnit = new QLabel(tr("second(s)") + " >>");
                this->buttonTriggerAutoStep = new QPushButton(tr("Launch"));
                this->buttonTriggerAutoStep->setCheckable(true);
                autoStepLayout->addWidget(autoStepBeginText);
                autoStepLayout->addWidget(this->autoStepValue);
                autoStepLayout->addWidget(autoStepUnit);
                autoStepLayout->addWidget(this->buttonTriggerAutoStep);

                this->buttonTriggerView->setCheckable(true);

                connect(buttonReset,                 &QPushButton::clicked, this, &SimulatorTab::reset);
                connect(buttonNextStep,              &QPushButton::clicked, this, &SimulatorTab::nextStep);
                connect(this->buttonTriggerView,     &QPushButton::clicked, this, &SimulatorTab::buttonTriggerViewClicked);
                connect(this->buttonTriggerAutoStep, &QPushButton::clicked, this, &SimulatorTab::buttonLauchAutoStepClicked);

                this->simulationTools->layout()->addWidget(this->buttonTriggerView);
                this->simulationTools->layout()->addWidget(buttonReset);
                this->simulationTools->layout()->addWidget(buttonNextStep);
                ((QVBoxLayout*)this->simulationTools->layout())->addLayout(autoStepLayout);

                if (this->machine->getInputs().count() != 0)
                {
                    QLabel* inputListTitle = new QLabel(tr("Click on bits from the list below to switch value:"));
                    inputListTitle->setAlignment(Qt::AlignCenter);
                    this->simulationTools->layout()->addWidget(inputListTitle);

                    inputList = new InputsSelector(this->machine->getInputs());
                    this->simulationTools->layout()->addWidget(inputList);
                }

                this->reset();

                this->timeLine = new SimulationWindow(this->machine, this->clock, 0);

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

    foreach(Signal* sig, machine->getAllVariables())
    {
        sig->reinitialize();
    }
}

void SimulatorTab::targetStateSelectionMade(QObject* choosenTransition)
{
    delete targetStateSelection;
    targetStateSelection = nullptr;
    delete signalMapper;
    signalMapper = nullptr;

    FsmTransition* actualTransition = (FsmTransition*)choosenTransition;

    if (actualTransition->getCondition() != nullptr)
        actualTransition->activateActions();

    currentState->setActive(false);

    currentState = actualTransition->getTarget();
    currentState->setActive(true);
}

void SimulatorTab::buttonTriggerViewClicked()
{
    if (this->buttonTriggerView->isChecked())
        this->buttonTriggerView->setText(tr("View machine"));
    else
        this->buttonTriggerView->setText(tr("View timeline"));


    emit triggerView();
}

void SimulatorTab::buttonLauchAutoStepClicked()
{
    if (this->buttonTriggerAutoStep->isChecked())
    {
        this->clock->start(this->autoStepValue->text().toInt() * 1000);
    }
    else
    {
        this->clock->stop();
    }
}

void SimulatorTab::clockEvent()
{
    QList<FsmTransition*> potentialTransitions;

    foreach(FsmTransition* transition, currentState->getOutgoingTransitions())
    {
        if (transition->getCondition() != nullptr)
        {
            if (transition->getCondition()->isTrue())
            {
                potentialTransitions.append(transition);
            }
        }
        else
        {
            potentialTransitions.append(transition);
        }
    }

    if (potentialTransitions.count() == 1)
    {
        if (potentialTransitions[0]->getCondition() != nullptr)
            potentialTransitions[0]->activateActions();

        currentState->setActive(false);

        currentState = potentialTransitions[0]->getTarget();
        currentState->setActive(true);

    }
    else if (potentialTransitions.count() > 1)
    {
        // If multiple transitions leading to the same state, no way to differentiate them,
        // but this is anyway a small instant patch, user should correct his machine.

        targetStateSelection = new QWidget();
        QVBoxLayout* choiceWindowLayout = new QVBoxLayout(targetStateSelection);

        QLabel* choiceWindowWarningText = new QLabel(tr("Warning! There are multiple active transitions going out the current state!") + "<br />"
                                                     + tr("This means your FSM is wrong by construction. This should be fixed.") + "<br />"
                                                     + tr("For current simulation, just choose the target state in the following list:"));

        choiceWindowLayout->addWidget(choiceWindowWarningText);

        signalMapper = new QSignalMapper();

        connect(signalMapper, static_cast<void (QSignalMapper::*)(QObject*)>(&QSignalMapper::mapped), this, &SimulatorTab::targetStateSelectionMade);

        foreach(FsmTransition* transition, potentialTransitions)
        {
            QPushButton* button = new QPushButton(transition->getTarget()->getName());

            signalMapper->setMapping(button, transition);

            connect(button, &QPushButton::clicked, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
            connect(button, &QPushButton::clicked, targetStateSelection, &QWidget::close);

            choiceWindowLayout->addWidget(button);
        }

        targetStateSelection->show();
    }
}
