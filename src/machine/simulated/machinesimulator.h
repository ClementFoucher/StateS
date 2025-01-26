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

#ifndef MACHINESIMULATOR_H
#define MACHINESIMULATOR_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>

// SateS classes
#include "statestypes.h"
class Clock;
class SimulatedComponent;


class MachineSimulator : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineSimulator();
	~MachineSimulator();

	/////
	// Object functions
public:
	virtual void build() = 0;

	shared_ptr<Clock> getClock() const;

	void reset();
	void doStep();
	void start(uint period);
	void suspend();

	const QList<SimulatedComponent*> getSimulatedComponents() const;
	SimulatedComponent* getComponent(componentId_t componentId) const;

	void setMemorizedStateActionBehavior     (SimulationBehavior_t behv);
	void setContinuousStateActionBehavior    (SimulationBehavior_t behv);
	void setMemorizedTransitionActionBehavior(SimulationBehavior_t behv);
	void setPulseTransitionActionBehavior    (SimulationBehavior_t behv);

	/////
	// Object variables
protected:
	QMap<componentId_t, SimulatedComponent*> simulatedComponents;

	shared_ptr<Clock> clock;

	SimulationBehavior_t memorizedStateActionBehavior;
	SimulationBehavior_t continuousStateActionBehavior;
	SimulationBehavior_t memorizedTransitionActionBehavior;
	SimulationBehavior_t pulseTransitionActionBehavior;

};

#endif // MACHINESIMULATOR_H
