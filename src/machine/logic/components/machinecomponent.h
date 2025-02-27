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

#ifndef MACHINECOMPONENT_H
#define MACHINECOMPONENT_H

// Parent
#include <QObject>

// StateS classes
#include "statestypes.h"


class MachineComponent : public QObject
{
	Q_OBJECT

	/////
	// Static functions
private:
	static componentId_t getUniqueId();

	/////
	// Constructors/destructors
public:
	explicit MachineComponent();
	explicit MachineComponent(componentId_t id);
	~MachineComponent();

	/////
	// Object functions
public:
	componentId_t getId() const;

signals:
	void componentEditedEvent(componentId_t componentId); // Triggered when logic object has been edited in a way that requires a graphic redraw
	void componentDeletedEvent(componentId_t componentId);

	/////
	// Object variables
protected:
	componentId_t id;

};

#endif // MACHINECOMPONENT_H
