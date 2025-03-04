/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "simulatortimecontroller.h"

// Qt classes
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>

// StateS classes
#include "machinemanager.h"
#include "simulatedmachine.h"


SimulatorTimeController::SimulatorTimeController(QWidget* parent) :
	QWidget(parent)
{
	auto simulator = machineManager->getMachineSimulator();
	if (simulator == nullptr) return;


	auto mainLayout = new QVBoxLayout();
	this->setLayout(mainLayout);

	auto buttonReset    = new QPushButton(tr("Reset"));

	auto stepLabel = new QLabel(tr("Use the ") + "<b>" + tr("Do one step") + "</b>" + tr(" button") + " " + tr("or launch a timer to do steps automatically:"));

	auto buttonNextStep = new QPushButton("> " + tr("Do one step") + " >");

	auto autoStepLayout = new QHBoxLayout();
	auto autoStepBeginText = new QLabel(">> " + tr("Do one step every"));
	this->autoStepValue = new QLineEdit("1");
	auto autoStepUnit = new QLabel(tr("second(s)") + " >>");
	this->buttonTriggerAutoStep = new QPushButton(tr("Launch"));
	this->buttonTriggerAutoStep->setCheckable(true);
	autoStepLayout->addWidget(autoStepBeginText);
	autoStepLayout->addWidget(this->autoStepValue);
	autoStepLayout->addWidget(autoStepUnit);
	autoStepLayout->addWidget(this->buttonTriggerAutoStep);

	connect(buttonReset,                 &QPushButton::clicked, simulator.get(), &SimulatedMachine::reset);
	connect(buttonNextStep,              &QPushButton::clicked, simulator.get(), &SimulatedMachine::doStep);
	connect(this->buttonTriggerAutoStep, &QPushButton::clicked, this,            &SimulatorTimeController::buttonLauchAutoStepClicked);

	connect(simulator.get(), &SimulatedMachine::autoSimulationToggledEvent, this, &SimulatorTimeController::autoSimulationToggledEventHandler);

	mainLayout->addWidget(buttonReset);
	mainLayout->addWidget(stepLabel);
	mainLayout->addWidget(buttonNextStep);
	mainLayout->addLayout(autoStepLayout);
}

void SimulatorTimeController::buttonLauchAutoStepClicked()
{
	auto simulator = machineManager->getMachineSimulator();
	if (simulator == nullptr) return;


	if (this->buttonTriggerAutoStep->isChecked())
	{
		float value = this->autoStepValue->text().toFloat() * 1000;
		if (value != 0)
		{
			simulator->start(value);
		}
		else
		{
			simulator->start(1000);
		}
	}
	else
	{
		simulator->suspend();
	}
}

void SimulatorTimeController::autoSimulationToggledEventHandler(bool started)
{
	if (started == true)
	{
		this->buttonTriggerAutoStep->setText(tr("Suspend"));
		this->buttonTriggerAutoStep->setChecked(true);
	}
	else
	{
		this->buttonTriggerAutoStep->setText(tr("Launch"));
		this->buttonTriggerAutoStep->setChecked(false);
	}
}
