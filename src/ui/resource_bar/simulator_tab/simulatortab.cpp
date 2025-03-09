/*
 * Copyright © 2014-2025 Clément Foucher
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
#include <QGroupBox>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "machinesimulator.h"
#include "contextmenu.h"
#include "simulatorconfigurator.h"
#include "simulatortimecontroller.h"
#include "inputsselector.h"


SimulatorTab::SimulatorTab(QWidget* parent) :
    QWidget(parent)
{
	this->boxLayout = new QVBoxLayout();
	this->setLayout(this->boxLayout);
	this->boxLayout->setAlignment(Qt::AlignTop);

	// Title

	auto title = new QLabel("<b>" + tr("Simulator") + "</b>", this);
	title->setAlignment(Qt::AlignCenter);
	this->boxLayout->addWidget(title);

	// Start button

	this->buttonTriggerSimulation = new QPushButton(tr("Start simulation"), this);
	this->buttonTriggerSimulation->setCheckable(true);
	connect(this->buttonTriggerSimulation, &QAbstractButton::clicked, this, &SimulatorTab::triggerSimulationMode);
	this->boxLayout->addWidget(this->buttonTriggerSimulation);

	// Configuration

	this->configurationGroup = new QGroupBox(tr("Simulation configuration"));
	auto configurationLayout = new QVBoxLayout(this->configurationGroup);
	this->boxLayout->addWidget(this->configurationGroup);

	this->simulatorConfigurator = new SimulatorConfigurator();
	configurationLayout->addWidget(this->simulatorConfigurator);
}

SimulatorTab::~SimulatorTab()
{
	// Close simulation
	this->triggerSimulationMode(false);
}

void SimulatorTab::triggerSimulationMode(bool enabled)
{
	if (enabled == true)
	{
		if (machineManager->getCurrentSimulationMode() == SimulationMode_t::editMode)
		{
			auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
			if (fsm == nullptr) return;


			if (fsm->getInitialStateId() != nullId)
			{
				// Enable simulation mode
				machineManager->setSimulationMode(SimulationMode_t::simulateMode);
				auto machineSimulator = machineManager->getMachineSimulator();
				if (machineSimulator == nullptr) return;


				// Set simulator configuration
				machineSimulator->setMemorizedStateActionBehavior     (this->simulatorConfigurator->getMemorizedStateActionBehavior());
				machineSimulator->setContinuousStateActionBehavior    (this->simulatorConfigurator->getContinuousStateActionBehavior());
				machineSimulator->setMemorizedTransitionActionBehavior(this->simulatorConfigurator->getMemorizedTransitionActionBehavior());
				machineSimulator->setPulseTransitionActionBehavior    (this->simulatorConfigurator->getPulseTransitionActionBehavior());

				// Update button text and hide configuration
				this->buttonTriggerSimulation->setText(tr("End simulation"));
				this->configurationGroup->setVisible(false);

				// Reset simulator to reset graphic part which have been created when setSimulationMode emited mode change event
				machineSimulator->reset();


				// Build time manager
				this->timeManagerGroup = new QGroupBox(tr("Time manager"));
				QVBoxLayout* timeManagerLayout = new QVBoxLayout(this->timeManagerGroup);

				auto simulationTimeManager = new SimulatorTimeController();
				timeManagerLayout->addWidget(simulationTimeManager);

				this->boxLayout->addWidget(this->timeManagerGroup);


				// Build inputs selector
				this->inputsGroup = new QGroupBox(tr("Inputs"));
				QVBoxLayout* inputsLayout = new QVBoxLayout(this->inputsGroup);

				auto inputList = new InputsSelector();
				inputsLayout->addWidget(inputList);

				this->boxLayout->addWidget(this->inputsGroup);
			}
			else
			{
				this->buttonTriggerSimulation->setChecked(false);

				ContextMenu* menu = ContextMenu::createErrorMenu(tr("No initial state!"));
				menu->popup(this->buttonTriggerSimulation->mapToGlobal(QPoint(this->buttonTriggerSimulation->width(), -menu->sizeHint().height())));
			}
		}
	}
	else
	{
		if (machineManager->getCurrentSimulationMode() == SimulationMode_t::simulateMode)
		{
			delete this->timeManagerGroup;
			this->timeManagerGroup = nullptr;

			delete this->inputsGroup;
			this->inputsGroup = nullptr;

			machineManager->setSimulationMode(SimulationMode_t::editMode);

			this->buttonTriggerSimulation->setText(tr("Start simulation"));
			this->configurationGroup->setVisible(true);
		}
	}
}
