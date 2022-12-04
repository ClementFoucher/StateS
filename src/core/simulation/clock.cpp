/*
 * Copyright © 2014-2016 Clément Foucher
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
#include "clock.h"

// Qt classes
#include <QTimer>


Clock::Clock() :
    QObject()
{
}

void Clock::start(uint intervalms)
{
	if (this->timer == nullptr)
	{
		this->timer = shared_ptr<QTimer>(new QTimer());
		connect(this->timer.get(), &QTimer::timeout, this, &Clock::nextStep);
	}

	this->timer->setInterval(intervalms);
	this->timer->start();
}

void Clock::stop()
{
	if (this->timer != nullptr)
	{
		this->timer->stop();
	}
}

void Clock::nextStep()
{
	emit prepareForClockEvent();
	emit clockEvent();
}

void Clock::reset()
{
	emit resetLogicEvent();
	emit resetGraphicEvent();
}
