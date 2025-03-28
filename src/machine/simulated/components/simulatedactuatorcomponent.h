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

#ifndef SIMULATEDACTUATORCOMPONENT_H
#define SIMULATEDACTUATORCOMPONENT_H

// Parent
#include "simulatedcomponent.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QList>

// StateS classes
class SimulatedActionOnVariable;


class SimulatedActuatorComponent : public SimulatedComponent
{
	/////
	// Constructors/destructors
public:
	explicit SimulatedActuatorComponent(componentId_t componentId);

	/////
	// Object functions
public:
	const QList<shared_ptr<SimulatedActionOnVariable>> getActions() const;

	void resetActions();

	/////
	// Object variables
private:
	QList<shared_ptr<SimulatedActionOnVariable>> actionList;

};

#endif // SIMULATEDACTUATORCOMPONENT_H
