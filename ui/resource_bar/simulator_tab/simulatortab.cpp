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
#include <QGroupBox>

// Debug
#include <QDebug>

// StateS classes
#include "fsmstate.h"
#include "fsm.h"
#include "fsmtransition.h"
#include "signal.h"
#include "simulationwidget.h"
#include "contextmenu.h"
#include "inputsselector.h"
#include "fsmsimulator.h"
#include "checkboxhtml.h"


SimulatorTab::SimulatorTab(shared_ptr<Fsm> machine, QWidget* parent) :
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

SimulationWidget* SimulatorTab::getTimeline() const
{
    return this->timeLine;
}

void SimulatorTab::triggerSimulationMode(bool enabled)
{
    shared_ptr<Fsm> machine = this->machine.lock();

    if (enabled)
    {
        if (this->simulationTools == nullptr)
        {
            if (machine->getInitialState() != nullptr)
            {
                // First thing to do would be to check machine correctness

                this->buttonTriggerSimulation->setText(tr("End simulation"));

                this->simulator = shared_ptr<FsmSimulator>(new FsmSimulator(machine));
                machine->setSimulator(this->simulator);

                this->simulationTools = new QWidget();
                this->simulationTools->setLayout(new QVBoxLayout());
                this->layout()->addWidget(this->simulationTools);

                //
                // Options
                QGroupBox* optionsGroup = new QGroupBox(tr("Options"));
                QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);

                this->buttonTriggerView  = new QPushButton(tr("View timeline"));
                this->buttonTriggerView->setCheckable(true);
                optionsLayout->addWidget(this->buttonTriggerView);
                connect(this->buttonTriggerView,     &QPushButton::clicked, this, &SimulatorTab::buttonTriggerViewClicked);

                this->checkBoxDelay = new CheckBoxHtml(tr("Add delay from clock rising edge to outputs events on timeline"));
                connect(this->checkBoxDelay, &CheckBoxHtml::toggled, this, &SimulatorTab::delayOutputOptionTriggeredEvent);
                optionsLayout->addWidget(this->checkBoxDelay);

                this->simulationTools->layout()->addWidget(optionsGroup);

                //
                // Time manager
                QGroupBox* timeManagerGroup = new QGroupBox(tr("Time manager"));
                QVBoxLayout* timeManagerLayout = new QVBoxLayout(timeManagerGroup);

                QPushButton* buttonReset    = new QPushButton(tr("Reset"));
                QPushButton* buttonNextStep = new QPushButton("> " + tr("Do one step") + " >");

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

                connect(buttonReset,                 &QPushButton::clicked, this->simulator.get(), &FsmSimulator::reset);
                connect(buttonNextStep,              &QPushButton::clicked, this->simulator.get(), &FsmSimulator::doStep);
                connect(this->buttonTriggerAutoStep, &QPushButton::clicked, this, &SimulatorTab::buttonLauchAutoStepClicked);

                timeManagerLayout->addWidget(buttonReset);
                timeManagerLayout->addWidget(buttonNextStep);
                timeManagerLayout->addLayout(autoStepLayout);

                this->simulationTools->layout()->addWidget(timeManagerGroup);

                //
                // Inputs
                QGroupBox* inputsGroup = new QGroupBox(tr("Inputs"));
                QVBoxLayout* inputsLayout = new QVBoxLayout(inputsGroup);

                shared_ptr<Fsm> machine = this->machine.lock();

                if ( (machine != nullptr) && (machine->getInputs().count() != 0) )
                {
                    QLabel* inputListHint = new QLabel(tr("Click on bits from the list below to switch value:"));
                    inputListHint->setAlignment(Qt::AlignCenter);
                    inputListHint->setWordWrap(true);
                    inputsLayout->addWidget(inputListHint);

                    inputList = new InputsSelector(machine->getInputs());
                    inputsLayout->addWidget(inputList);
                }

                this->simulationTools->layout()->addWidget(inputsGroup);

                this->timeLine = new SimulationWidget(this, machine, this->simulator->getClock());
                this->timeLine->show();

                emit beginSimulationEvent();
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

            this->machine.lock()->setSimulator(nullptr);
            this->simulator.reset();

            foreach(shared_ptr<FsmState> state, machine->getStates())
            {
                state->setActive(false);
            }

            this->buttonTriggerSimulation->setText(tr("Start simulation"));

            emit endSimulationEvent();
        }
    }
}

void SimulatorTab::buttonTriggerViewClicked()
{
    if (this->buttonTriggerView->isChecked())
        this->buttonTriggerView->setText(tr("View machine"));
    else
        this->buttonTriggerView->setText(tr("View timeline"));


    emit triggerViewRequestEvent();
}

void SimulatorTab::buttonLauchAutoStepClicked()
{
    if (this->buttonTriggerAutoStep->isChecked())
    {
        float value = this->autoStepValue->text().toFloat() * 1000;
        if (value != 0)
            this->simulator->start(value);
        else
            this->simulator->start(1000);

        this->buttonTriggerAutoStep->setText(tr("Suspend"));
    }
    else
    {
        //this->clock->stop();
        this->simulator->suspend();
        this->buttonTriggerAutoStep->setText(tr("Launch"));
    }
}
