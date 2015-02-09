/*
 * Copyright © 2014 Clément Foucher
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

#include <QHBoxLayout>

#include "inputbitselector.h"

#include "signal.h"

InputBitSelector::InputBitSelector(Signal* signalToCommand, uint bitNumber, QWidget *parent) :
    QFrame(parent)
{
    this->signalToCommand = signalToCommand;
    this->bitNumber       = bitNumber;

    QHBoxLayout* layout = new QHBoxLayout(this);

    this->bitValue = new QLabel(QString::number(this->signalToCommand->getCurrentValue()[this->bitNumber]));
    this->bitValue->setToolTip(tr("Bit") + " " + QString::number(this->bitNumber) + " " + tr("of signal") + " " + this->signalToCommand->getName());
    layout->addWidget(this->bitValue);

    connect(this->signalToCommand, &Signal::signalStateChangedEvent, this, &InputBitSelector::signalValueChanged);
}

void InputBitSelector::enterEvent(QEvent* event)
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
    LogicValue signalValue = this->signalToCommand->getCurrentValue();

    signalValue[this->bitNumber] = !signalValue[this->bitNumber];

    this->signalToCommand->setCurrentValue(signalValue);
}

void InputBitSelector::signalValueChanged()
{
    this->bitValue->setText(QString::number(this->signalToCommand->getCurrentValue()[this->bitNumber]));
}
