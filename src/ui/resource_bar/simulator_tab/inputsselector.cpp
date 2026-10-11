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
#include "inputsselector.h"

// Qt
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>

// StateS
#include "machinemanager.h"
#include "machine.h"
#include "simulatedmachine.h"
#include "simulatedvariable.h"
#include "inputbitselector.h"
#include "inputbooleanselector.h"
#include "inputlitteralvalueselector.h"


InputsSelector::InputsSelector(QWidget* parent) :
	QWidget(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;


	auto mainLayout = new QVBoxLayout(this);
	mainLayout->setAlignment(Qt::AlignTop);

	auto inputIds = machine->getInputVariablesIds();
	if (inputIds.count() != 0)
	{
		auto inputListHint = new QLabel(tr("Click on framed values below to change input values:"));
		inputListHint->setAlignment(Qt::AlignCenter);
		inputListHint->setWordWrap(true);
		mainLayout->addWidget(inputListHint);

		auto scrollArea = new QScrollArea();
		scrollArea->setWidgetResizable(true);
		mainLayout->addWidget(scrollArea);

		auto scrollAreaWidget = new QWidget();
		auto scrollAreaWidgetLayout = new QVBoxLayout(scrollAreaWidget);
		scrollArea->setWidget(scrollAreaWidget);

		for (auto& inputId : inputIds)
		{
			auto simulatedVariable = simulatedMachine->getSimulatedVariable(inputId);
			if (simulatedVariable == nullptr) continue;


			auto variableLayout = new QHBoxLayout();
			scrollAreaWidgetLayout->addLayout(variableLayout);

			auto variableName = new QLabel(simulatedVariable->getName());
			variableLayout->addWidget(variableName);

			variableLayout->addStretch();

			switch (simulatedVariable->getType())
			{
			case MachineValue::Type_t::bitVector:
			{
				auto bitLayout = new QHBoxLayout();
				variableLayout->addLayout(bitLayout);

				uint bitVectorSize = simulatedVariable->getCurrentValue().getBitVectorValue().getSize();
				for (int bitIndex = static_cast<int>(bitVectorSize)-1 ; bitIndex >= 0 ; bitIndex--)
				{
					auto currentBit = new InputBitSelector(inputId, bitIndex);
					bitLayout->addWidget(currentBit);
				}
				break;
			}
			case MachineValue::Type_t::boolean:
				variableLayout->addWidget(new InputBooleanSelector(inputId));
				break;
			case MachineValue::Type_t::natural:
			case MachineValue::Type_t::relative:
				variableLayout->addWidget(new InputLitteralValueSelector(inputId));
				break;
			case MachineValue::Type_t::nullType:
				break;
			}
		}

		scrollAreaWidgetLayout->addStretch();
	}
	else
	{
		auto noInputHint = new QLabel("<i>" + tr("No input available on the machine") +"</i>");
		noInputHint->setAlignment(Qt::AlignCenter);
		noInputHint->setWordWrap(true);
		mainLayout->addWidget(noInputHint);
	}
}
