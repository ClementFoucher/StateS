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

#ifndef FSMSIMULATOR_H
#define FSMSIMULATOR_H

// Parent
#include "machinesimulator.h"

// C++ classes
#include <memory>
using namespace std;

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

private slots:
	void resetEventHandler();
	void clockEventHandler();

private:
	void activateTransition(componentId_t transitionId);

	/////
	// Object variables
private:
	componentId_t activeStateId;
	componentId_t latestTransitionCrossedId;
	QMap<uint, componentId_t> potentialTransitionsIds;

	QWidget* targetStateSelector = nullptr;
	QSignalMapper* signalMapper  = nullptr; // Use pointer because we need a deleteLater instruction

};

#endif // FSMSIMULATOR_H
