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

#ifndef SIMULATEDFSMTRANSITION_H
#define SIMULATEDFSMTRANSITION_H

// Parent
#include "simulatedactuatorcomponent.h"

// Stdlib
#include <memory>

// StateS
class SimulatedEquation;


class SimulatedFsmTransition : public SimulatedActuatorComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedFsmTransition(ComponentId logicComponentId);

	/////
	// Object functions
public:
	std::shared_ptr<SimulatedEquation> getCondition();

	ComponentId getSourceStateId() const;
	ComponentId getTargetStateId() const;

private slots:
	void conditionChangedEventHandler();

	/////
	// Object variable
private:
	ComponentId sourceStateId = nullId;
	ComponentId targetStateId = nullId;

	std::shared_ptr<SimulatedEquation> condition;

};

#endif // SIMULATEDFSMTRANSITION_H

