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

#ifndef SIMULATEDFSM_H
#define SIMULATEDFSM_H

// Parent
#include "simulatedmachine.h"

// Qt classes
#include <QMap>
class QDialog;
class QSignalMapper;

// StateS classes
#include "statestypes.h"
class SimulatedFsmState;
class SimulatedFsmTransition;


class SimulatedFsm : public SimulatedMachine
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedFsm();

	/////
	// Object functions
public:
	virtual void build() override;

	shared_ptr<SimulatedFsmState>      getSimulatedState     (componentId_t componentId) const;
	shared_ptr<SimulatedFsmTransition> getSimulatedTransition(componentId_t componentId) const;

	void forceStateActivation(componentId_t stateToActivate);

	componentId_t getInitialStateId() const;
	componentId_t getActiveStateId()  const;

private slots:
	void targetStateSelectionMadeEventHandler(int i);

private:
	virtual void subcomponentReset()          override;
	virtual void subcomponentPrepareStep()    override;
	virtual void subcomponentPrepareActions() override;
	virtual void subcomponentDoStep()         override;

	void crossTransition(componentId_t transitionId);

	void activateStateActions(componentId_t actuatorId, bool isFirstActivation);
	void activateTransitionActions(componentId_t actuatorId, bool isPreparation);

	void deactivateStateActions(componentId_t actuatorId);
	void deactivateTransitionActions(componentId_t actuatorId, bool isPreparation);

	/////
	// Signals
signals:
	void stateChangedEvent();

	/////
	// Object variables
private:
	componentId_t initialStateId            = nullId;
	componentId_t activeStateId             = nullId;
	componentId_t latestTransitionCrossedId = nullId;
	componentId_t transitionToBeCrossedId   = nullId;
	QMap<uint, componentId_t> potentialTransitionsIds;

	QDialog* targetStateSelector = nullptr;
	QSignalMapper* signalMapper  = nullptr;

};

#endif // SIMULATEDFSM_H
