/*
 * Copyright © 2014-2024 Clément Foucher
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
#include <QVBoxLayout>
#include <QLineEdit>
#include <QGroupBox>

// Debug
#include <QDebug>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "contextmenu.h"
#include "inputsselector.h"
#include "machinesimulator.h"


SimulatorTab::SimulatorTab(QWidget* parent) :
    QWidget(parent)
{
	this->setLayout(new QVBoxLayout());
	this->layout()->setAlignment(Qt::AlignTop);

	QLabel* title = new QLabel("<b>" + tr("Simulator") + "</b>", this);
	title->setAlignment(Qt::AlignCenter);
	this->layout()->addWidget(title);

	this->buttonTriggerSimulation = new QPushButton(tr("Start simulation"), this);
	this->buttonTriggerSimulation->setCheckable(true);
	connect(this->buttonTriggerSimulation, &QAbstractButton::clicked, this, &SimulatorTab::triggerSimulationMode);
	this->layout()->addWidget(this->buttonTriggerSimulation);
}

SimulatorTab::~SimulatorTab()
{
	// Close simulation
	this->triggerSimulationMode(false);
}

void SimulatorTab::triggerSimulationMode(bool enabled)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	if (enabled == true)
	{
		if (this->simulationTools == nullptr)
		{
			if (fsm->getInitialStateId() != 0)
			{
				// First thing to do would be to check machine correctness

				this->buttonTriggerSimulation->setText(tr("End simulation"));

				machineManager->setSimulationMode(SimulationMode_t::simulateMode);
				auto simulator = machineManager->getMachineSimulator();
				if (simulator == nullptr) return;

				// Reset simulator to reset graphic part which have been created when setSimulator emited mode change event
				simulator->reset();

				this->simulationTools = new QWidget();
				this->simulationTools->setLayout(new QVBoxLayout());
				this->layout()->addWidget(this->simulationTools);

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

				connect(buttonReset,                 &QPushButton::clicked, simulator.get(), &MachineSimulator::reset);
				connect(buttonNextStep,              &QPushButton::clicked, simulator.get(), &MachineSimulator::doStep);
				connect(this->buttonTriggerAutoStep, &QPushButton::clicked, this,            &SimulatorTab::buttonLauchAutoStepClicked);

				timeManagerLayout->addWidget(buttonReset);
				timeManagerLayout->addWidget(buttonNextStep);
				timeManagerLayout->addLayout(autoStepLayout);

				this->simulationTools->layout()->addWidget(timeManagerGroup);

				//
				// Inputs
				QGroupBox* inputsGroup = new QGroupBox(tr("Inputs"));
				QVBoxLayout* inputsLayout = new QVBoxLayout(inputsGroup);

				if (fsm->getInputs().count() != 0)
				{
					QLabel* inputListHint = new QLabel(tr("Click on bits from the list below to switch value:"));
					inputListHint->setAlignment(Qt::AlignCenter);
					inputListHint->setWordWrap(true);
					inputsLayout->addWidget(inputListHint);

					this->inputList = new InputsSelector(fsm->getInputs());
					inputsLayout->addWidget(this->inputList);
				}

				this->simulationTools->layout()->addWidget(inputsGroup);
			}
			else
			{
				this->buttonTriggerSimulation->setChecked(false);

				ContextMenu* menu = ContextMenu::createErrorMenu(tr("No initial state!"));
				menu->popup(this->buttonTriggerSimulation->mapToGlobal(QPoint(this->buttonTriggerSimulation->width(), -menu->sizeHint().height())));
			}
		}
		else
		{
			qDebug() << "(SimulatorTab:) Warning: Trying to begin simulation while already launched.";
			qDebug() << "Command ignored.";
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

			machineManager->setSimulationMode(SimulationMode_t::editMode);

			this->buttonTriggerSimulation->setText(tr("Start simulation"));
		}
	}
}

void SimulatorTab::buttonLauchAutoStepClicked()
{
	auto simulator = machineManager->getMachineSimulator();
	if (simulator == nullptr) return;

	if (this->buttonTriggerAutoStep->isChecked())
	{
		float value = this->autoStepValue->text().toFloat() * 1000;
		if (value != 0)
			simulator->start(value);
		else
			simulator->start(1000);

		this->buttonTriggerAutoStep->setText(tr("Suspend"));
	}
	else
	{
		simulator->suspend();
		this->buttonTriggerAutoStep->setText(tr("Launch"));
	}
}
