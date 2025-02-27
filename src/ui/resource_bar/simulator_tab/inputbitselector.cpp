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
#include "inputbitselector.h"

// C++classes
#include <memory>
using namespace std;

// Qt classes
#include <QHBoxLayout>
#include <QStyle>
#include <QLabel>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"


InputBitSelector::InputBitSelector(componentId_t variableToCommandId, uint bitNumber, QWidget *parent) :
    QFrame(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variableToCommand = machine->getVariable(variableToCommandId);
	if (variableToCommand == nullptr) return;


	this->variableToCommandId = variableToCommandId;
	this->bitNumber           = bitNumber;

	QHBoxLayout* layout = new QHBoxLayout(this);

	this->bitValue = new QLabel(QString::number(variableToCommand->getCurrentValue()[this->bitNumber]), this);
	this->bitValue->setToolTip(tr("Bit") + " " + QString::number(this->bitNumber) + " " + tr("of variable") + " " + variableToCommand->getName());
	layout->addWidget(this->bitValue);

	this->setMinimumHeight(this->bitValue->sizeHint().height() + 2*this->style()->pixelMetric(QStyle::PM_LayoutTopMargin) + 2);

	connect(variableToCommand.get(), &Variable::variableCurrentValueChangedEvent, this, &InputBitSelector::variableValueChangedEventHandler);
}

void InputBitSelector::enterEvent(QEnterEvent* event)
{
	this->setStyleSheet("InputBitSelector {border: 1px solid black; border-radius: 10px}");

	QFrame::enterEvent(event);
}


void InputBitSelector::leaveEvent(QEvent* event)
{
	this->setStyleSheet("");

	QFrame::leaveEvent(event);
}

void InputBitSelector::mousePressEvent(QMouseEvent*)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variableToCommand = machine->getVariable(variableToCommandId);
	if (variableToCommand == nullptr) return;


	LogicValue variableValue = variableToCommand->getCurrentValue();

	variableValue[this->bitNumber] = !variableValue[this->bitNumber];

	variableToCommand->setCurrentValue(variableValue);
}

void InputBitSelector::mouseMoveEvent(QMouseEvent*)
{
	// Just because this class never sends event to parent
}

void InputBitSelector::mouseReleaseEvent(QMouseEvent*)
{
	// Just because this class never sends event to parent
}

void InputBitSelector::mouseDoubleClickEvent(QMouseEvent*)
{
	// Just because this class never sends event to parent
}

void InputBitSelector::variableValueChangedEventHandler()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variableToCommand = machine->getVariable(variableToCommandId);
	if (variableToCommand == nullptr) return;


	this->bitValue->setText(QString::number(variableToCommand->getCurrentValue()[this->bitNumber]));
}
