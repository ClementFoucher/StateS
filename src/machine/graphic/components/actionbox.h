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

#ifndef ACTIONBOX_H
#define ACTIONBOX_H

// Parent
#include <QGraphicsItemGroup>

// Qt classes
#include <QGraphicsPathItem>

// StateS classes
#include "statestypes.h"


class ActionBox : public QGraphicsItemGroup
{
	//Q_OBJECT

	/////
	// Static variables
private:
	static const QPen defaultPen;

	/////
	// Constructors/destructors
public:
	explicit ActionBox(componentId_t actuatorId, bool addLine = false);

	/////
	// Object functions
public:
	void refreshDisplay();
	qreal getHeight() const;

private:
	void buildActionBox();

	/////
	// Object variables
protected:
	qreal textHeight = 0;
	componentId_t actuatorId = 0;
	bool addLine;
	QGraphicsPathItem* actionsOutline = nullptr;
	QGraphicsLineItem* leftLine       = nullptr;

};

#endif // ACTIONBOX_H
