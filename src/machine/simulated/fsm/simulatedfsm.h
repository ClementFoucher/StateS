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

#ifndef SIMULATEDFSM_H
#define SIMULATEDFSM_H

// Parent
#include "simulatedmachine.h"

// Qt
#include <QMap>
class QDialog;
class QSignalMapper;

// StateS
class SimulatedFsmState;
class SimulatedFsmTransition;


class SimulatedFsm : public SimulatedMachine
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedFsm() = default;

	/////
	// Object functions
public:
	virtual void build() override;

	shared_ptr<SimulatedFsmState>      getSimulatedState     (ComponentId componentId) const;
	shared_ptr<SimulatedFsmTransition> getSimulatedTransition(ComponentId componentId) const;

	void forceStateActivation(ComponentId stateToActivate);

	ComponentId getInitialStateId() const;
	ComponentId getActiveStateId()  const;

private slots:
	void targetStateSelectionMadeEventHandler(int i);

private:
	virtual void subMachineReset()          override;
	virtual void subMachinePrepareStep()    override;
	virtual void subMachinePrepareActions() override;
	virtual void subMachineDoStep()         override;

	/////
	// Signals
signals:
	void stateChangedEvent();

	/////
	// Object variables
private:
	// Static state
	ComponentId initialStateId = nullId;

	// Dynamic state
	ComponentId activeStateId = nullId;

	// Temporary working variables
	ComponentId transitionToBeCrossedId = nullId;
	QList<ComponentId> variablesToResetBeforeNextStep;
	QList<ComponentId> variablesToResetAfterNextStep;

	// Resolution of transition conflict
	QMap<uint, ComponentId> potentialTransitionsIds;
	QDialog* targetStateSelector = nullptr;
	QSignalMapper* signalMapper  = nullptr;

};

#endif // SIMULATEDFSM_H
