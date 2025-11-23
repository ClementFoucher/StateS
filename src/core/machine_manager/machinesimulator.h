/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MACHINESIMULATOR_H
#define MACHINESIMULATOR_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QTimer;

// SateS classes
#include "statestypes.h"
class SimulatedMachine;


class MachineSimulator : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineSimulator();

	/////
	// Object functions
public:
	void initialize();

	void reset();
	void doStep();
	void start(uint period);
	void suspend();

	void setMemorizedStateActionBehavior     (SimulationBehavior_t behv);
	void setContinuousStateActionBehavior    (SimulationBehavior_t behv);
	void setMemorizedTransitionActionBehavior(SimulationBehavior_t behv);
	void setPulseTransitionActionBehavior    (SimulationBehavior_t behv);

	shared_ptr<SimulatedMachine> getSimulatedMachine() const;

private slots:
	void timerTimeoutEventHandler();

	void emergencyShutDownEventHandler();
	void resumeNormalActivitiesEventHandler();

	/////
	// Signals
signals:
	void timelineDoStepEvent();
	void timelineResetEvent();

	void autoSimulationToggledEvent(bool simulating);

	/////
	// Object variables
private:
	shared_ptr<SimulatedMachine> simulatedMachine;
	shared_ptr<QTimer> timer;
	bool emergencyShutDown = false;
	bool wasAutoSimulatingBeforeShutDown;

};

#endif // MACHINESIMULATOR_H
