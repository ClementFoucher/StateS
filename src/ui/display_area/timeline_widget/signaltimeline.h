/*
 * Copyright © 2014-2023 Clément Foucher
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

#ifndef SIGNALTIMELINE_H
#define SIGNALTIMELINE_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Signal;
class Clock;
class GraphicTimeLine;
class TimelineWidget;


class SignalTimeline : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SignalTimeline(uint delay, TimelineWidget* simulationWidget, shared_ptr<Signal> signal, shared_ptr<Clock> clock, QWidget* parent = nullptr);

	/////
	// Object functions
private slots:
	void clockEventHandler();
	void resetEventHandler();
	void updateCurrentValue();
	void updateDelayOutputOption(uint delay);

	/////
	// Object variables
private:
	weak_ptr<Signal> signal;

	QList<GraphicTimeLine*> signalLineDisplay;

};

#endif // SIGNALTIMELINE_H
