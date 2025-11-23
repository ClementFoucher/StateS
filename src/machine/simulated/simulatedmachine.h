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

#ifndef SIMULATEDMACHINE_H
#define SIMULATEDMACHINE_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>

// SateS classes
#include "statestypes.h"
class SimulatedComponent;
class SimulatedActuatorComponent;
class SimulatedVariable;


class SimulatedMachine : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedMachine() = default;

	/////
	// Object functions
public:
	virtual void build();

	shared_ptr<SimulatedActuatorComponent> getSimulatedActuatorComponent(componentId_t actuatorId) const;
	shared_ptr<SimulatedVariable>          getSimulatedVariable         (componentId_t variableId) const;

	void reset();
	void prepareStep();
	void prepareActions();
	void doStep();

	void setMemorizedStateActionBehavior     (SimulationBehavior_t behv);
	void setContinuousStateActionBehavior    (SimulationBehavior_t behv);
	void setMemorizedTransitionActionBehavior(SimulationBehavior_t behv);
	void setPulseTransitionActionBehavior    (SimulationBehavior_t behv);

protected:
	void registerSimulatedComponent(componentId_t componentId, shared_ptr<SimulatedComponent> component);
	shared_ptr<SimulatedComponent> getSimulatedComponent(componentId_t componentId) const;

private:
	virtual void subMachineReset()          = 0;
	virtual void subMachinePrepareStep()    = 0;
	virtual void subMachinePrepareActions() = 0;
	virtual void subMachineDoStep()         = 0;

	/////
	// Signals
signals:
	void simulatedComponentUpdatedEvent(componentId_t componentId);
	void emergencyShutDownEvent();
	void resumeNormalActivitiesEvent();

	/////
	// Object variables
protected:
	SimulationBehavior_t memorizedStateActionBehavior;
	SimulationBehavior_t continuousStateActionBehavior;
	SimulationBehavior_t memorizedTransitionActionBehavior;
	SimulationBehavior_t pulseTransitionActionBehavior;

private:
	QMap<componentId_t, shared_ptr<SimulatedComponent>> simulatedComponents;

};

#endif // SIMULATEDMACHINE_H
