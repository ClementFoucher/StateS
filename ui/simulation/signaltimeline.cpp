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

#include "signaltimeline.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "signal.h"
#include "clock.h"
#include "graphictimeline.h"


SignalTimeline::SignalTimeline(Signal* signal, Clock* clock, bool dynamic, QWidget* parent) :
    QWidget(parent)
{
    this->variable = signal;
    this->clock = clock;
    this->isDynamic = dynamic;

    QHBoxLayout* globalLayout = new QHBoxLayout(this);

    QLabel* varName = new QLabel(signal->getName());
    globalLayout->addWidget(varName);

    if (signal->getSize() == 1)
    {
        GraphicTimeLine* timeLineDisplay = new GraphicTimeLine(signal->getInitialValue()[0]);
        timeLineDisplay->setMinimumHeight(20);
        timeLineDisplay->setMaximumHeight(20);
        variableLineDisplay.append(timeLineDisplay);

        globalLayout->addWidget(timeLineDisplay);
    }
    else
    {
        QVBoxLayout* bitsLayout = new QVBoxLayout();

        for (uint i = 0 ; i < signal->getSize() ; i++)
        {
            QHBoxLayout* innerLayout = new QHBoxLayout();

            QLabel* bitNumber = new QLabel(QString::number(i));
            innerLayout->addWidget(bitNumber);

            GraphicTimeLine* timeLineDisplay = new GraphicTimeLine(signal->getInitialValue()[i]);
            timeLineDisplay->setMinimumHeight(20);
            timeLineDisplay->setMaximumHeight(20);
            variableLineDisplay.append(timeLineDisplay);
            innerLayout->addWidget(timeLineDisplay);

            bitsLayout->addLayout(innerLayout);
        }

        globalLayout->addLayout(bitsLayout);
    }

    if (isDynamic)
    {
        connect(signal, &Signal::signalStateChangedEvent, this, &SignalTimeline::prepareClockEvent);
    }

    connect(clock, &Clock::clockEvent, this, &SignalTimeline::clockEvent);
    connect(clock, &Clock::resetEvent, this, &SignalTimeline::resetEvent);
}

void SignalTimeline::clockEvent()
{
    for (uint i = 0 ; i < variable->getSize() ; i++)
    {
        variableLineDisplay[i]->addPointConst();
        variableLineDisplay[i]->addPoint(variable->getCurrentValue()[i]);
    }
}

// This is used for input, because state may have changed since previous clock
void SignalTimeline::prepareClockEvent()
{
    for (uint i = 0 ; i < variable->getSize() ; i++)
    {
        variableLineDisplay[i]->updateLastPoint(variable->getCurrentValue()[i]);
    }
}

void SignalTimeline::resetEvent()
{
    for (uint i = 0 ; i < variable->getSize() ; i++)
    {
        variableLineDisplay[i]->reset(variable->getInitialValue()[i]);
    }
}
