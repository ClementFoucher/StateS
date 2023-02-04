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

#ifndef GRAPHICACTUATOR_H
#define GRAPHICACTUATOR_H

// Parent
#include "graphiccomponent.h"

// Qt classes
class QGraphicsItemGroup;

// StateS classes
#include "statestypes.h"


class GraphicActuator : public GraphicComponent
{
	Q_OBJECT

	/////
	// Static variables
private:
	static const QPen defaultPen;

	/////
	// Constructors/destructors
public:
	explicit GraphicActuator(componentId_t logicComponentId);
	~GraphicActuator();

	/////
	// Object functions
public:
	QGraphicsItemGroup* getActionsBox() const;

	virtual void refreshDisplay() override;

protected:

private:
	void buildActionsBox();

	virtual void updateActionBoxPosition() = 0;

	/////
	// Object variables
protected:
	QGraphicsItemGroup* actionsBox = nullptr;

};

#endif // GRAPHICACTUATOR_H
