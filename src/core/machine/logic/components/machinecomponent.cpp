/*
 * Copyright © 2014-2020 Clément Foucher
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


MachineComponent::MachineComponent(shared_ptr<Machine> owningMachine)
{
	this->owningMachine = owningMachine;
}

/**
 * @brief MachineComponent::getOwningMachine
 * @return Owning machine as a shared pointer.
 * We can lock it as if the component still exists,
 * the machine still exists and there should be no
 * calls to getting owning machine concurrent with
 * machine deletion.
 */
shared_ptr<Machine> MachineComponent::getOwningMachine() const
{
	return this->owningMachine.lock();
}
