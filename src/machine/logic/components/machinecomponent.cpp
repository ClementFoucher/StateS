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

// Current class header
#include "machinecomponent.h"


componentId_t MachineComponent::getUniqueId()
{
	static componentId_t currentId = 0L;

	// ID O is reserved for nullId,
	// increment *before* assigning ID.
	currentId++;
	return currentId;
}

MachineComponent::MachineComponent()
{
	this->id = MachineComponent::getUniqueId();
}

MachineComponent::MachineComponent(componentId_t id)
{
	this->id = id;
}

MachineComponent::~MachineComponent()
{
	emit this->componentDeletedEvent(this->id);
}

componentId_t MachineComponent::getId() const
{
	return this->id;
}
