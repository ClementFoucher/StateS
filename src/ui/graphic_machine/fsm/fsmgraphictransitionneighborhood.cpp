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

// Current class header
#include "fsmgraphictransitionneighborhood.h"

// StateS classes
#include "fsmgraphicstate.h"
#include "fsmgraphictransition.h"


FsmGraphicTransitionNeighborhood::FsmGraphicTransitionNeighborhood(FsmGraphicState* source, FsmGraphicState* target)
{
	this->source = source;
	this->target = target;
}

void FsmGraphicTransitionNeighborhood::insertTransition(FsmGraphicTransition* transition)
{
	map.insert(map.count(), transition);
}

void FsmGraphicTransitionNeighborhood::removeTransition(FsmGraphicTransition* transition)
{
	int removedValueKey = map.key(transition);
	map.remove(removedValueKey);

	// Set all following transition to go down one position to keep a consistent key system
	for (int currentKey = removedValueKey+1 ; currentKey <= map.lastKey() ; currentKey++)
	{
		map.insert(currentKey-1, map[currentKey]);
		map.remove(currentKey);
	}
}

QList<FsmGraphicTransition*> FsmGraphicTransitionNeighborhood::getTransitions() const
{
	return map.values();
}

FsmGraphicState* FsmGraphicTransitionNeighborhood::getSource() const
{
	return this->source;
}

FsmGraphicState* FsmGraphicTransitionNeighborhood::getTarget() const
{
	return this->target;
}

int FsmGraphicTransitionNeighborhood::count() const
{
	return map.count();
}

int FsmGraphicTransitionNeighborhood::getTransitionNumber(FsmGraphicTransition* transition) const
{
	return map.key(transition, 0);
}
