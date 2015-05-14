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
#include "signaltimeline.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>

// StateS classes
#include "signal.h"
#include "clock.h"
#include "graphictimeline.h"
#include "simulationwidget.h"
#include "output.h"


SignalTimeline::SignalTimeline(uint outputDelay, SimulationWidget *simulationWidget, shared_ptr<Signal> signal, shared_ptr<Clock> clock, bool dynamic, QWidget* parent) :
    QWidget(parent)
{
    // If this parameter is not null, this is only for this connection
    if (simulationWidget != nullptr)
        connect(simulationWidget, &SimulationWidget::outputDelayChangedEvent, this, &SignalTimeline::updateDelayOutputOption);

    this->signal      = signal;
    this->isDynamic   = dynamic;

    QHBoxLayout* globalLayout = new QHBoxLayout(this);

    QLabel* varName = new QLabel(signal->getName());
    globalLayout->addWidget(varName);

    if (signal->getSize() == 1)
    {
        GraphicTimeLine* timeLineDisplay = new GraphicTimeLine(4, outputDelay, signal->getInitialValue()[0]);
        timeLineDisplay->setMinimumHeight(20);
        timeLineDisplay->setMaximumHeight(20);
        this->signalLineDisplay.append(timeLineDisplay);

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

            GraphicTimeLine* timeLineDisplay = new GraphicTimeLine(4, outputDelay, signal->getInitialValue()[i]);
            timeLineDisplay->setMinimumHeight(20);
            timeLineDisplay->setMaximumHeight(20);
            this->signalLineDisplay.append(timeLineDisplay);
            innerLayout->addWidget(timeLineDisplay);

            bitsLayout->addLayout(innerLayout);
        }

        globalLayout->addLayout(bitsLayout);
    }

    if (isDynamic)
    {
        connect(signal.get(), &Signal::signalDynamicStateChangedEvent, this, &SignalTimeline::prepareClockEventHandler);
    }

    connect(clock.get(), &Clock::clockEvent, this, &SignalTimeline::clockEventHandler);
    connect(clock.get(), &Clock::resetEvent, this, &SignalTimeline::resetEventHandler);
}

void SignalTimeline::clockEventHandler()
{
    shared_ptr<Signal> signal = this->signal.lock();

    if (signal != nullptr)
    {
        for (uint i = 0 ; i < signal->getSize() ; i++)
        {
            this->signalLineDisplay[i]->addPoint(signal->getCurrentValue()[i]);
        }
    }
}

// This is used for input, because state may have changed since previous clock
void SignalTimeline::prepareClockEventHandler()
{
    shared_ptr<Signal> signal = this->signal.lock();

    if (signal != nullptr)
    {
        for (uint i = 0 ; i < signal->getSize() ; i++)
        {
            this->signalLineDisplay[i]->updateLastPoint(signal->getCurrentValue()[i]);
        }
    }
}

void SignalTimeline::resetEventHandler()
{
    shared_ptr<Signal> signal = this->signal.lock();

    if (signal != nullptr)
    {
        for (uint i = 0 ; i < signal->getSize() ; i++)
        {
            this->signalLineDisplay[i]->reset(signal->getInitialValue()[i]);
        }
    }
}

void SignalTimeline::updateDelayOutputOption(uint delay)
{
    foreach (GraphicTimeLine* gtl, this->signalLineDisplay)
    {
        gtl->chageEventDelay(delay);
    }
}
