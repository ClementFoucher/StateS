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

#ifndef FSMSIMULATOR_H
#define FSMSIMULATOR_H

// Parent
#include "machinesimulator.h"

// Qt classes
#include <QMap>
class QWidget;
class QSignalMapper;

// StateS classes
#include "statestypes.h"


class FsmSimulator : public MachineSimulator
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmSimulator();

	/////
	// Object functions
public:
	virtual void build() override;

	void targetStateSelectionMadeEventHandler(int i);
	void forceStateActivation(componentId_t stateToActivate);
	componentId_t getActiveStateId() const;

private slots:
	void resetEventHandler();
	void clockAboutToTickEventHandler();
	void clockPrepareActionsEventHandler();
	void clockEventHandler();

private:
	void crossTransition(componentId_t transitionId);

	void activateStateActions(componentId_t actuatorId, bool isFirstActivation);
	void activateTransitionActions(componentId_t actuatorId, bool isPreparation);

	void deactivateStateActions(componentId_t actuatorId);
	void deactivateTransitionActions(componentId_t actuatorId, bool isPreparation);

signals:
	void stateChangedEvent();

	/////
	// Object variables
private:
	componentId_t activeStateId             = nullId;
	componentId_t latestTransitionCrossedId = nullId;
	componentId_t transitionToBeCrossedId   = nullId;
	QMap<uint, componentId_t> potentialTransitionsIds;

	QWidget* targetStateSelector = nullptr;
	QSignalMapper* signalMapper  = nullptr; // Use pointer because we need a deleteLater instruction

};

#endif // FSMSIMULATOR_H
