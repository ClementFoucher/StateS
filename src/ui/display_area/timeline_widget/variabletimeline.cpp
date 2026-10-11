/*
 * Copyright © 2014-2026 Clément Foucher
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
#include "variabletimeline.h"

// Qt
#include <QLabel>
#include <QVBoxLayout>

// StateS
#include "machinemanager.h"
#include "machinesimulator.h"
#include "simulatedmachine.h"
#include "simulatedvariable.h"
#include "graphicbittimeline.h"
#include "graphicvectortimeline.h"


VariableTimeline::VariableTimeline(uint outputDelay, ComponentId variableId, QWidget* parent) :
	QWidget(parent)
{
	auto machineSimulator = machineManager->getMachineSimulator();
	if (machineSimulator == nullptr) return;

	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(variableId);
	if (simulatedVariable == nullptr) return;

	auto initialValue = simulatedVariable->getInitialValue();
	if (initialValue.getType() == MachineValue::Type_t::nullType) return;


	this->variableId = variableId;

	// Default: display a vector value and no individual bits
	bool hasVectorValue = true;
	int individualBitsCount = 0;

	// Adapt default values
	if (simulatedVariable->getType() == MachineValue::Type_t::boolean)
	{
		// Booleans have no vector value and 1 individual bit
		hasVectorValue = false;
		individualBitsCount = 1;
	}
	else if (simulatedVariable->getType() == MachineValue::Type_t::bitVector)
	{
		// Bit vectors have at least one individual bit
		individualBitsCount = initialValue.getBitVectorValue().getSize();

		if (individualBitsCount == 1)
		{
			// Do not display vector value for 1-bit vectors
			hasVectorValue = false;
		}
	}

	//
	// Build content
	auto globalLayout = new QHBoxLayout(this);

	// Variable name
	auto varName = new QLabel(simulatedVariable->getName());
	globalLayout->addWidget(varName);

	// Add a vertical line for vectors
	if (individualBitsCount > 1)
	{
		auto line = new QFrame();
		line->setFrameShape(QFrame::VLine);
		line->setFrameShadow(QFrame::Plain);
		globalLayout->addWidget(line);
	}

	// Build timeline(s) layout

	auto timelinesLayout = new QVBoxLayout();

	// Global value sub-layout
	if (hasVectorValue == true)
	{
		auto globalValueLayout = new QHBoxLayout();

		// Add a global "Value" label for vectors
		if (individualBitsCount > 1)
		{
			auto valueLabel = new QLabel(tr("Value"));
			globalValueLayout->addWidget(valueLabel);
		}

		// Timeline
		this->vectorValueDisplay = new GraphicVectorTimeLine(outputDelay, initialValue);
		this->vectorValueDisplay->setMinimumHeight(30);
		this->vectorValueDisplay->setMaximumHeight(30);
		globalValueLayout->addWidget(this->vectorValueDisplay);

		timelinesLayout->addLayout(globalValueLayout);
	}

	// Individual bits sub-layouts
	for (int i = 0 ; i < individualBitsCount ; i++)
	{
		auto currentBitLayout = new QHBoxLayout();

		// Add a bit label for vectors
		if (individualBitsCount > 1)
		{
			auto bitNumberLabel = new QLabel(tr("Bit") + " #" + QString::number(i));
			currentBitLayout->addWidget(bitNumberLabel);
		}

		// Timeline
		GraphicBitTimeLine* timeLineDisplay = nullptr;

		if (simulatedVariable->getType() == MachineValue::Type_t::boolean)
		{
			timeLineDisplay = new GraphicBitTimeLine(outputDelay, initialValue.getBooleanValue());
		}
		else // (simulatedVariable->getType() == MachineValue::Type_t::bitVector)
		{
			timeLineDisplay = new GraphicBitTimeLine(outputDelay, initialValue.getBitVectorValue()[i]);
		}

		timeLineDisplay->setMinimumHeight(20);
		timeLineDisplay->setMaximumHeight(20);
		this->individualBitsDisplay.append(timeLineDisplay);
		currentBitLayout->addWidget(timeLineDisplay);

		timelinesLayout->addLayout(currentBitLayout);
	}

	globalLayout->addLayout(timelinesLayout);

	connect(simulatedVariable.get(), &SimulatedVariable::variableCurrentValueChangedEvent, this, &VariableTimeline::updateCurrentValue);

	connect(machineSimulator.get(), &MachineSimulator::timelineDoStepEvent, this, &VariableTimeline::doStepEventHandler);
	connect(machineSimulator.get(), &MachineSimulator::timelineResetEvent,  this, &VariableTimeline::resetEventHandler);
}

// On clock event, duplicate current value:
// it will be edited dynamically with variable update
void VariableTimeline::doStepEventHandler()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(variableId);
	if (simulatedVariable == nullptr) return;

	if (simulatedVariable->getType() == MachineValue::Type_t::nullType) return;


	if (this->vectorValueDisplay != nullptr)
	{
		this->vectorValueDisplay->addPoint(simulatedVariable->getCurrentValue());
	}

	for (uint currentBit = 0 ; currentBit < this->individualBitsDisplay.count() ; currentBit++)
	{
		auto currentTimeLine = this->individualBitsDisplay[currentBit];
		if (currentTimeLine != nullptr)
		{
			if (simulatedVariable->getType() == MachineValue::Type_t::boolean)
			{
				currentTimeLine->addPoint(simulatedVariable->getCurrentValue().getBooleanValue());
			}
			else // (simulatedVariable->getType() == MachineValue::Type_t::bitVector)
			{
				currentTimeLine->addPoint(simulatedVariable->getCurrentValue().getBitVectorValue()[currentBit]);
			}
		}
	}
}

// Value is updated depending on actions on variable
void VariableTimeline::updateCurrentValue()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(variableId);
	if (simulatedVariable == nullptr) return;

	if (simulatedVariable->getType() == MachineValue::Type_t::nullType) return;


	if (this->vectorValueDisplay != nullptr)
	{
		this->vectorValueDisplay->updateLastPoint(simulatedVariable->getCurrentValue());
	}

	for (uint currentBit = 0 ; currentBit < this->individualBitsDisplay.count() ; currentBit++)
	{
		auto currentTimeLine = this->individualBitsDisplay[currentBit];
		if (currentTimeLine != nullptr)
		{
			if (simulatedVariable->getType() == MachineValue::Type_t::boolean)
			{
				currentTimeLine->updateLastPoint(simulatedVariable->getCurrentValue().getBooleanValue());
			}
			else // (simulatedVariable->getType() == MachineValue::Type_t::bitVector)
			{
				currentTimeLine->updateLastPoint(simulatedVariable->getCurrentValue().getBitVectorValue()[currentBit]);
			}
		}
	}
}

void VariableTimeline::resetEventHandler()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(variableId);
	if (simulatedVariable == nullptr) return;

	if (simulatedVariable->getType() == MachineValue::Type_t::nullType) return;


	if (this->vectorValueDisplay != nullptr)
	{
		this->vectorValueDisplay->reset(simulatedVariable->getCurrentValue());
	}

	for (uint currentBit = 0 ; currentBit < this->individualBitsDisplay.count() ; currentBit++)
	{
		auto currentTimeLine = this->individualBitsDisplay[currentBit];
		if (currentTimeLine != nullptr)
		{
			if (simulatedVariable->getType() == MachineValue::Type_t::boolean)
			{
				currentTimeLine->reset(simulatedVariable->getCurrentValue().getBooleanValue());
			}
			else // (simulatedVariable->getType() == MachineValue::Type_t::bitVector)
			{
				currentTimeLine->reset(simulatedVariable->getCurrentValue().getBitVectorValue()[currentBit]);
			}
		}
	}
}
