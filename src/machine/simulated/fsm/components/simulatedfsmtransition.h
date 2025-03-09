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

#ifndef SIMULATEDFSMTRANSITION_H
#define SIMULATEDFSMTRANSITION_H

// Parent
#include "simulatedactuatorcomponent.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
class SimulatedEquation;


class SimulatedFsmTransition : public SimulatedActuatorComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedFsmTransition(componentId_t logicComponentId);

	/////
	// Object functions
public:
	shared_ptr<SimulatedEquation> getCondition();

	componentId_t getSourceStateId() const;
	componentId_t getTargetStateId() const;

private slots:
	void conditionChangedEventHandler();

	/////
	// Object variable
private:
	componentId_t sourceStateId = nullId;
	componentId_t targetStateId = nullId;

	shared_ptr<SimulatedEquation> condition;

};

#endif // SIMULATEDFSMTRANSITION_H

