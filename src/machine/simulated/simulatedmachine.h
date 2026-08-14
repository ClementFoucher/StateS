/*
 * Copyright © 2014-2026 Clément Foucher
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

// Stdlib
#include <memory>

// Qt
#include <QMap>

// StateS
#include "componentid.h"
#include "machinesimulator.h"
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

	std::shared_ptr<SimulatedActuatorComponent> getSimulatedActuatorComponent(ComponentId actuatorId) const;
	std::shared_ptr<SimulatedVariable>          getSimulatedVariable         (ComponentId variableId) const;

	void reset();
	void prepareStep();
	void prepareActions();
	void doStep();

	void setMemorizedStateActionBehavior     (MachineSimulator::SimulationBehavior_t behv);
	void setContinuousStateActionBehavior    (MachineSimulator::SimulationBehavior_t behv);
	void setMemorizedTransitionActionBehavior(MachineSimulator::SimulationBehavior_t behv);
	void setPulseTransitionActionBehavior    (MachineSimulator::SimulationBehavior_t behv);

protected:
	void registerSimulatedComponent(ComponentId componentId, std::shared_ptr<SimulatedComponent> component);
	std::shared_ptr<SimulatedComponent> getSimulatedComponent(ComponentId componentId) const;

private:
	virtual void subMachineReset()          = 0;
	virtual void subMachinePrepareStep()    = 0;
	virtual void subMachinePrepareActions() = 0;
	virtual void subMachineDoStep()         = 0;

	/////
	// Signals
signals:
	void simulatedComponentUpdatedEvent(ComponentId componentId);
	void emergencyShutDownEvent();
	void resumeNormalActivitiesEvent();

	/////
	// Object variables
protected:
	MachineSimulator::SimulationBehavior_t memorizedStateActionBehavior;
	MachineSimulator::SimulationBehavior_t continuousStateActionBehavior;
	MachineSimulator::SimulationBehavior_t memorizedTransitionActionBehavior;
	MachineSimulator::SimulationBehavior_t pulseTransitionActionBehavior;

private:
	QMap<ComponentId, std::shared_ptr<SimulatedComponent>> simulatedComponents;

};

#endif // SIMULATEDMACHINE_H
