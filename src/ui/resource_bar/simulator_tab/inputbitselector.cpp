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

// Qt classes
#include <QHBoxLayout>
#include <QStyle>
#include <QLabel>

// StateS classes
#include "variable.h"


InputBitSelector::InputBitSelector(shared_ptr<Variable> signalToCommand, uint bitNumber, QWidget *parent) :
    QFrame(parent)
{
	this->signalToCommand = signalToCommand;
	this->bitNumber       = bitNumber;

	QHBoxLayout* layout = new QHBoxLayout(this);

	this->bitValue = new QLabel(QString::number(signalToCommand->getCurrentValue()[this->bitNumber]), this);
	this->bitValue->setToolTip(tr("Bit") + " " + QString::number(this->bitNumber) + " " + tr("of signal") + " " + signalToCommand->getName());
	layout->addWidget(this->bitValue);

	this->setMinimumHeight(this->bitValue->sizeHint().height() + 2*this->style()->pixelMetric(QStyle::PM_LayoutTopMargin) + 2);

	connect(signalToCommand.get(), &Variable::signalDynamicStateChangedEvent, this, &InputBitSelector::signalValueChangedEventHandler);
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
	shared_ptr<Variable> l_signalToCommand = this->signalToCommand.lock();
	if (l_signalToCommand != nullptr)
	{
		LogicValue signalValue = l_signalToCommand->getCurrentValue();

		signalValue[this->bitNumber] = !signalValue[this->bitNumber];

		l_signalToCommand->setCurrentValue(signalValue);  // Throws StatesException - TODO: what if signalValue[] is incorrect?
	}
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

void InputBitSelector::signalValueChangedEventHandler()
{
	shared_ptr<Variable> l_signalToCommand = this->signalToCommand.lock();
	if (l_signalToCommand != nullptr)
	{
		this->bitValue->setText(QString::number(l_signalToCommand->getCurrentValue()[this->bitNumber]));
	}
}
