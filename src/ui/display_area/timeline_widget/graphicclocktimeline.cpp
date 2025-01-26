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
#include "graphicclocktimeline.h"

// StateS classes
#include "clock.h"


GraphicClockTimeLine::GraphicClockTimeLine(shared_ptr<Clock> clock, QWidget* parent) :
	GraphicBitTimeLine(0, false, parent)
{
	connect(clock.get(), &Clock::clockUpdateTimelineEvent, this, &GraphicClockTimeLine::clockEventHandler);
	connect(clock.get(), &Clock::resetGraphicEvent,        this, &GraphicClockTimeLine::resetEventHandler);

	this->pointsPerCycle = 2;
}

void GraphicClockTimeLine::clockEventHandler()
{
	this->addPoint(true);
	this->addPoint(false);
}

void GraphicClockTimeLine::resetEventHandler()
{
	this->reset(false);
}

