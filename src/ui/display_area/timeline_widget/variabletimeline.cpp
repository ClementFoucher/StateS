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


VariableTimeline::VariableTimeline(uint outputDelay, componentId_t variableId, QWidget* parent) :
	QWidget(parent)
{
	auto machineSimulator = machineManager->getMachineSimulator();
	if (machineSimulator == nullptr) return;

	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(variableId);
	if (simulatedVariable == nullptr) return;


	auto initialValue = simulatedVariable->getInitialValue();
	int variableSize;
	switch (simulatedVariable->getType())
	{
	case MachineValue::Type_t::boolean:
		variableSize = 1;
		break;
	case MachineValue::Type_t::bitVector:
		variableSize = initialValue.getBitVectorValue().getSize();
		break;
	case MachineValue::Type_t::nullType:
		return;
	}


	this->variableId = variableId;

	auto globalLayout = new QHBoxLayout(this);

	auto varName = new QLabel(simulatedVariable->getName());
	globalLayout->addWidget(varName);

	auto bitsLayout = new QVBoxLayout();

	// Global value display for vectors
	if (variableSize > 1)
	{
		auto line = new QFrame();
		line->setFrameShape(QFrame::VLine);
		line->setFrameShadow(QFrame::Plain);
		globalLayout->addWidget(line);

		auto innerLayout = new QHBoxLayout();

		auto valueLabel = new QLabel(tr("Value"));
		innerLayout->addWidget(valueLabel);

		auto timeLineDisplay = new GraphicVectorTimeLine(outputDelay, initialValue.getBitVectorValue());
		timeLineDisplay->setMinimumHeight(30);
		timeLineDisplay->setMaximumHeight(30);
		this->variableLineDisplay.append(timeLineDisplay);
		innerLayout->addWidget(timeLineDisplay);

		bitsLayout->addLayout(innerLayout);
	}

	// Individual bits display
	for (int i = 0 ; i < variableSize ; i++)
	{
		auto innerLayout = new QHBoxLayout();

		GraphicBitTimeLine* timeLineDisplay = nullptr;
		switch (simulatedVariable->getType())
		{
		case MachineValue::Type_t::boolean:
			timeLineDisplay = new GraphicBitTimeLine(outputDelay, initialValue.getBooleanValue());
			break;
		case MachineValue::Type_t::bitVector:
		{
			if (variableSize > 1)
			{
				auto bitNumberLabel = new QLabel(tr("Bit") + " #" + QString::number(i));
				innerLayout->addWidget(bitNumberLabel);
			}

			timeLineDisplay = new GraphicBitTimeLine(outputDelay, initialValue.getBitVectorValue()[i]);
			break;
		}
		case MachineValue::Type_t::nullType:
			// Checked before: should not happen
			break;
		}

		timeLineDisplay->setMinimumHeight(20);
		timeLineDisplay->setMaximumHeight(20);
		this->variableLineDisplay.append(timeLineDisplay);
		innerLayout->addWidget(timeLineDisplay);

		bitsLayout->addLayout(innerLayout);
	}
	globalLayout->addLayout(bitsLayout);

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


	uint bitNumber = 0;
	for (uint i = 0 ; i < this->variableLineDisplay.count() ; i++)
	{
		if ( (i == 0) &&
		     ( (simulatedVariable->getType() == MachineValue::Type_t::bitVector) && (simulatedVariable->getInitialValue().getBitVectorValue().getSize() > 1) )
		   )
		{
			auto vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->variableLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->addPoint(simulatedVariable->getCurrentValue().getBitVectorValue());
			}
		}
		else
		{
			auto timeLine = dynamic_cast<GraphicBitTimeLine*>(this->variableLineDisplay[i]);
			if (timeLine != nullptr)
			{
				switch (simulatedVariable->getType())
				{
				case MachineValue::Type_t::boolean:
					timeLine->addPoint(simulatedVariable->getCurrentValue().getBooleanValue());
					break;
				case MachineValue::Type_t::bitVector:
					timeLine->addPoint(simulatedVariable->getCurrentValue().getBitVectorValue()[bitNumber]);
					break;
				case MachineValue::Type_t::nullType:
					// Checked in constructor: should not happen
					break;
				}
			}
			bitNumber++;
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


	uint bitNumber = 0;
	for (uint i = 0 ; i < this->variableLineDisplay.count() ; i++)
	{
		if ( (i == 0) &&
		     ( (simulatedVariable->getType() == MachineValue::Type_t::bitVector) && (simulatedVariable->getInitialValue().getBitVectorValue().getSize() > 1) )
		   )
		{
			auto vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->variableLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->updateLastPoint(simulatedVariable->getCurrentValue().getBitVectorValue());
			}
		}
		else
		{
			auto timeLine = dynamic_cast<GraphicBitTimeLine*>(this->variableLineDisplay[i]);
			if (timeLine != nullptr)
			{
				switch (simulatedVariable->getType())
				{
				case MachineValue::Type_t::boolean:
					timeLine->updateLastPoint(simulatedVariable->getCurrentValue().getBooleanValue());
					break;
				case MachineValue::Type_t::bitVector:
					timeLine->updateLastPoint(simulatedVariable->getCurrentValue().getBitVectorValue()[bitNumber]);
					break;
				case MachineValue::Type_t::nullType:
					// Checked in constructor: should not happen
					break;
				}
			}
			bitNumber++;
		}
	}
}

void VariableTimeline::resetEventHandler()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(variableId);
	if (simulatedVariable == nullptr) return;


	uint bitNumber = 0;
	for (uint i = 0 ; i < this->variableLineDisplay.count() ; i++)
	{
		if ( (i == 0) &&
		     ( (simulatedVariable->getType() == MachineValue::Type_t::bitVector) && (simulatedVariable->getInitialValue().getBitVectorValue().getSize() > 1) )
		   )
		{
			auto vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->variableLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->reset(simulatedVariable->getCurrentValue().getBitVectorValue());
			}
		}
		else
		{
			auto timeLine = dynamic_cast<GraphicBitTimeLine*>(this->variableLineDisplay[i]);
			if (timeLine != nullptr)
			{
				switch (simulatedVariable->getType())
				{
				case MachineValue::Type_t::boolean:
					timeLine->reset(simulatedVariable->getCurrentValue().getBooleanValue());
					break;
				case MachineValue::Type_t::bitVector:
					timeLine->reset(simulatedVariable->getCurrentValue().getBitVectorValue()[bitNumber]);
					break;
				case MachineValue::Type_t::nullType:
					// Checked in constructor: should not happen
					break;
				}
			}
			bitNumber++;
		}
	}
}
