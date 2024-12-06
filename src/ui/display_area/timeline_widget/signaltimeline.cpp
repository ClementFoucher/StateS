/*
 * Copyright © 2014-2024 Clément Foucher
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
#include "StateS_signal.h"
#include "clock.h"
#include "graphicbittimeline.h"
#include "graphicvectortimeline.h"


SignalTimeline::SignalTimeline(uint outputDelay, shared_ptr<Signal> signal, shared_ptr<Clock> clock, QWidget* parent) :
    QWidget(parent)
{
	this->signal = signal;

	QHBoxLayout* globalLayout = new QHBoxLayout(this);

	QLabel* varName = new QLabel(signal->getName());
	globalLayout->addWidget(varName);

	QVBoxLayout* bitsLayout = new QVBoxLayout();

	// Global value display for vectors
	if (signal->getSize() > 1)
	{
		auto line = new QFrame();
		line->setFrameShape(QFrame::VLine);
		line->setFrameShadow(QFrame::Plain);
		globalLayout->addWidget(line);

		QHBoxLayout* innerLayout = new QHBoxLayout();

		QLabel* valueLabel = new QLabel(tr("Value"));
		innerLayout->addWidget(valueLabel);

		GraphicVectorTimeLine* timeLineDisplay = new GraphicVectorTimeLine(outputDelay, signal->getInitialValue());
		timeLineDisplay->setMinimumHeight(30);
		timeLineDisplay->setMaximumHeight(30);
		this->signalLineDisplay.append(timeLineDisplay);
		innerLayout->addWidget(timeLineDisplay);

		bitsLayout->addLayout(innerLayout);
	}

	// Individual bits display
	for (uint i = 0 ; i < signal->getSize() ; i++)
	{
		QHBoxLayout* innerLayout = new QHBoxLayout();

		if (signal->getSize() > 1)
		{
			QLabel* bitNumberLabel = new QLabel(tr("Bit") + " #" + QString::number(i));
			innerLayout->addWidget(bitNumberLabel);
		}

		GraphicBitTimeLine* timeLineDisplay = new GraphicBitTimeLine(outputDelay, signal->getInitialValue()[i]);
		timeLineDisplay->setMinimumHeight(20);
		timeLineDisplay->setMaximumHeight(20);
		this->signalLineDisplay.append(timeLineDisplay);
		innerLayout->addWidget(timeLineDisplay);

		bitsLayout->addLayout(innerLayout);
	}
	globalLayout->addLayout(bitsLayout);

	connect(signal.get(), &Signal::signalDynamicStateChangedEvent, this, &SignalTimeline::updateCurrentValue);

	connect(clock.get(), &Clock::prepareForClockEvent, this, &SignalTimeline::clockEventHandler);
	connect(clock.get(), &Clock::resetGraphicEvent,    this, &SignalTimeline::resetEventHandler);
}

// On clock event, duplicate current value:
// it will be edited dynamically with signal update
void SignalTimeline::clockEventHandler()
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal == nullptr) return;


	uint bitNumber = 0;
	for (uint i = 0 ; i < this->signalLineDisplay.count() ; i++)
	{
		if ( (l_signal->getSize() > 1) && (i == 0) )
		{
			GraphicVectorTimeLine* vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->signalLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->addPoint(l_signal->getCurrentValue());
			}
		}
		else
		{
			GraphicBitTimeLine* timeLine = dynamic_cast<GraphicBitTimeLine*>(this->signalLineDisplay[i]);
			if (timeLine != nullptr)
			{
				timeLine->addPoint(l_signal->getCurrentValue()[bitNumber]);
			}
			bitNumber++;
		}
	}
}

// Value is updated depending on actions on signal
void SignalTimeline::updateCurrentValue()
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal == nullptr) return;

	uint bitNumber = 0;
	for (uint i = 0 ; i < this->signalLineDisplay.count() ; i++)
	{
		if ( (l_signal->getSize() > 1) && (i == 0) )
		{
			GraphicVectorTimeLine* vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->signalLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->updateLastPoint(l_signal->getCurrentValue());
			}
		}
		else
		{
			GraphicBitTimeLine* timeLine = dynamic_cast<GraphicBitTimeLine*>(this->signalLineDisplay[i]);
			if (timeLine != nullptr)
			{
				timeLine->updateLastPoint(l_signal->getCurrentValue()[bitNumber]);
			}
			bitNumber++;
		}
	}
}

void SignalTimeline::resetEventHandler()
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal == nullptr) return;

	uint bitNumber = 0;
	for (uint i = 0 ; i < this->signalLineDisplay.count() ; i++)
	{
		if ( (l_signal->getSize() > 1) && (i == 0) )
		{
			GraphicVectorTimeLine* vectorTimeLine = dynamic_cast<GraphicVectorTimeLine*>(this->signalLineDisplay[0]);
			if (vectorTimeLine != nullptr)
			{
				vectorTimeLine->reset(l_signal->getCurrentValue());
			}
		}
		else
		{
			GraphicBitTimeLine* timeLine = dynamic_cast<GraphicBitTimeLine*>(this->signalLineDisplay[i]);
			if (timeLine != nullptr)
			{
				timeLine->reset(l_signal->getCurrentValue()[bitNumber]);
			}
			bitNumber++;
		}
	}
}
