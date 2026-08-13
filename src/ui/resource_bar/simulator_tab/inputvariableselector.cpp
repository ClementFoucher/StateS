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
#include "inputvariableselector.h"

// Qt
#include <QHBoxLayout>
#include <QLabel>

// StateS
#include "machinemanager.h"
#include "simulatedmachine.h"
#include "simulatedvariable.h"
#include "inputbitselector.h"
#include "inputbooleanselector.h"


InputVariableSelector::InputVariableSelector(componentId_t variableId, QWidget *parent) :
	QWidget(parent)
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(variableId);
	if (simulatedVariable == nullptr) return;


	auto globalLayout = new QHBoxLayout(this);

	auto variableName = new QLabel(simulatedVariable->getName());
	globalLayout->addWidget(variableName);

	globalLayout->addStretch();

	switch (simulatedVariable->getType())
	{
	case MachineValue::Type_t::bitVector:
	{
		auto bitLayout = new QHBoxLayout();
		globalLayout->addLayout(bitLayout);

		uint bitVectorSize = simulatedVariable->getCurrentValue().getBitVectorValue().getSize();
		for (int bitIndex = static_cast<int>(bitVectorSize)-1 ; bitIndex >= 0 ; bitIndex--)
		{
			auto currentBit = new InputBitSelector(variableId, bitIndex);
			bitLayout->addWidget(currentBit);
		}
		break;
	}
	case MachineValue::Type_t::boolean:
	{
		auto booleanSelector = new InputBooleanSelector(variableId);
		globalLayout->addWidget(booleanSelector);
		break;
	}
	case MachineValue::Type_t::nullType:
		break;
	}
}
