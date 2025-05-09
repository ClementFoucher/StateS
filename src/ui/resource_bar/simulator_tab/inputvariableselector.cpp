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
#include "inputvariableselector.h"

// Qt classes
#include <QHBoxLayout>
#include <QLabel>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "inputbitselector.h"


InputVariableSelector::InputVariableSelector(componentId_t relatedVariableId, QWidget *parent) :
    QWidget(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto relatedVariable = machine->getVariable(relatedVariableId);
	if (relatedVariable == nullptr) return;


	QHBoxLayout* globalLayout = new QHBoxLayout(this);

	QLabel* variableName = new QLabel(relatedVariable->getName(), this);
	globalLayout->addWidget(variableName);

	QHBoxLayout* bitLayout = new QHBoxLayout();
	globalLayout->addLayout(bitLayout, 0);

	for (int i = (int)relatedVariable->getSize()-1 ; i >= 0 ; i--)
	{
		InputBitSelector* currentBit = new InputBitSelector(relatedVariableId, i, this);
		bitLayout->addWidget(currentBit, 0, Qt::AlignRight);
	}
}
