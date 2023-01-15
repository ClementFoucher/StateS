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

#ifndef GRAPHICCOMPONENT_H
#define GRAPHICCOMPONENT_H

// Parent
#include <QObject>

// StateS classes
#include "statestypes.h"


class GraphicComponent : public QObject
{
	Q_OBJECT

	/////
	// Static variables
protected:
	static const QPen selectionPen;

	/////
	// Constructors/destructors
public:
	explicit GraphicComponent(componentId_t logicComponentId);

	/////
	// Object functions
public:
	componentId_t getLogicComponentId() const;

	virtual void refreshDisplay() = 0;

	/////
	// Object variables
protected:
	componentId_t logicComponentId;

};

#endif // GRAPHICCOMPONENT_H
