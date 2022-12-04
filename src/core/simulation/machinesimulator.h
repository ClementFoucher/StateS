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

#ifndef MACHINESIMULATOR_H
#define MACHINESIMULATOR_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// SateS classes
class Clock;


class MachineSimulator : public QObject
{
	Q_OBJECT

public:
	explicit MachineSimulator();

	virtual void enableOutputDelay(bool enable) = 0;
	virtual shared_ptr<Clock> getClock() const = 0;

signals:
	void outputDelayChangedEvent(bool enable);
};

#endif // MACHINESIMULATOR_H
