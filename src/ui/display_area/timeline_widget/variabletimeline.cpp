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
#include "variabletimeline.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>

// StateS classes
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


	this->variableId = variableId;

	QHBoxLayout* globalLayout = new QHBoxLayout(this);

	QLabel* varName = new QLabel(simulatedVariable->getName());
	globalLayout->addWidget(varName);

	QVBoxLayout* bitsLayout = new QVBoxLayout();

	// Global value display for vectors
	if (simulatedVariable->getSize() > 1)
	{
		auto line = new QFrame();
		line->setFrameShape(QFrame::VLine);
		line->setFrameShadow(QFrame::Plain);
		globalLayout->addWidget(line);

		QHBoxLayout* innerLayout = new QHBoxLayout();

		QLabel* valueLabel = new QLabel(tr("Value"));
		innerLayout->addWidget(valueLabel);

		GraphicVectorTimeLine* timeLineDisplay = new GraphicVectorTimeLine(outputDelay, simulatedVariable->getCurrentValue());
		timeLineDisplay->setMinimumHeight(30);
		timeLineDisplay->setMaximumHeight(30);
		this->variableLineDisplay.append(timeLineDisplay);
		innerLayout->addWidget(timeLineDisplay);

		bitsLayout->addLayout(innerLayout);
	}

	// Individual bits display
	for (uint i = 0 ; i < simulatedVariable->getSize() ; i++)
	{
		QHBoxLayout* innerLayout = new QHBoxLayout();

		if (simulatedVariable->getSize() > 1)
		{
			QLabel* bitNumberLabel = new QLabel(tr("Bit") + " #" + QString::number(i));
			innerLayout->addWidget(bitNumberLabel);
		}

		GraphicBitTimeLine* timeLineDisplay = new GraphicBitTimeLine(outputDelay, simulatedVariable->getCurrentValue()[i]);
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
		if ( (simulatedVariable->getSize() > 1) && (i == 0) )
		{
			GraphicVectorTimeLine* vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->variableLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->addPoint(simulatedVariable->getCurrentValue());
			}
		}
		else
		{
			GraphicBitTimeLine* timeLine = dynamic_cast<GraphicBitTimeLine*>(this->variableLineDisplay[i]);
			if (timeLine != nullptr)
			{
				timeLine->addPoint(simulatedVariable->getCurrentValue()[bitNumber]);
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
		if ( (simulatedVariable->getSize() > 1) && (i == 0) )
		{
			GraphicVectorTimeLine* vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->variableLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->updateLastPoint(simulatedVariable->getCurrentValue());
			}
		}
		else
		{
			GraphicBitTimeLine* timeLine = dynamic_cast<GraphicBitTimeLine*>(this->variableLineDisplay[i]);
			if (timeLine != nullptr)
			{
				timeLine->updateLastPoint(simulatedVariable->getCurrentValue()[bitNumber]);
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
		if ( (simulatedVariable->getSize() > 1) && (i == 0) )
		{
			GraphicVectorTimeLine* vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->variableLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->reset(simulatedVariable->getCurrentValue());
			}
		}
		else
		{
			GraphicBitTimeLine* timeLine = dynamic_cast<GraphicBitTimeLine*>(this->variableLineDisplay[i]);
			if (timeLine != nullptr)
			{
				timeLine->reset(simulatedVariable->getCurrentValue()[bitNumber]);
			}
			bitNumber++;
		}
	}
}
