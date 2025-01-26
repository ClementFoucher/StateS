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

#ifndef FSMSIMULATEDTRANSITION_H
#define FSMSIMULATEDTRANSITION_H

// Parents
#include "fsmgraphictransition.h"
#include "simulatedcomponent.h"

// StateS classes
#include "statestypes.h"


class FsmSimulatedTransition : public FsmGraphicTransition, public SimulatedComponent
{
	Q_OBJECT

	/////
	// Static variables
private:
	static const QPen inactivePen;
	static const QPen activePen;

	/////
	// Constructors/destructors
public:
	explicit FsmSimulatedTransition(componentId_t logicComponentId);

	virtual ~FsmSimulatedTransition() override;

	/////
	// Object functions
public:
	virtual void refreshDisplay() override;

};

#endif // FSMSIMULATEDTRANSITION_H

