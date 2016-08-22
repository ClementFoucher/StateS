/*
 * Copyright © 2014-2015 Clément Foucher
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
#include "inputsignalselector.h"

// Qt classes
#include <QHBoxLayout>
#include <QLabel>

// StateS classes
#include "StateS_signal.h"
#include "inputbitselector.h"


InputSignalSelector::InputSignalSelector(shared_ptr<Signal> relatedSignal, QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout* globalLayout = new QHBoxLayout(this);

    QLabel* signalName = new QLabel(relatedSignal->getName());
    globalLayout->addWidget(signalName);

    QHBoxLayout* bitLayout = new QHBoxLayout();
    globalLayout->addLayout(bitLayout, 0);

    for (int i = (int)relatedSignal->getSize()-1 ; i >= 0 ; i--)
    {
        InputBitSelector* currentBit = new InputBitSelector(relatedSignal, i);
        bitLayout->addWidget(currentBit, 0, Qt::AlignRight);
    }
}
