/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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

#ifndef STATESSCENE_H
#define STATESSCENE_H

// Parent
#include <QGraphicsScene>


class StatesScene : public QGraphicsScene
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit StatesScene() = default;
	virtual ~StatesScene() = default;

	/////
	// Object functions
public:
	QRectF getItemsBoundingRect(uint margin = 0) const;

protected:
	QGraphicsTextItem* buildGraphicsTextItem(const QString& text) const;

};

#endif // STATESSCENE_H
