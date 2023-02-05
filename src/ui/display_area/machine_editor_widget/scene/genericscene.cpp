/*
 * Copyright © 2014-2021 Clément Foucher
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
#include "genericscene.h"

// Qt classes
#include <QGraphicsItem>


GenericScene::GenericScene()
{

}

void GenericScene::setDisplaySize(const QSize& newSize)
{
	this->displaySize = newSize;
	this->updateSceneRect();
}

/**
 * @brief GenericScene::updateSceneRect
 * This function is called on resize, and
 * when mouse button release (can be item moved).
 * Not dynamically linked to item moving event,
 * as it caused too much difficulty with asynchonous
 * events. Mutex did no good as it desynchronized item
 * position and mouse position.
 *
 * Scene size is at least size of the display view,
 * But can be bigger is machine is bigger
 */
void GenericScene::updateSceneRect()
{
	// Initially, set scene to be size of display, centered on point 0
	// As this is this base view that will be extended,
	// Point (0, 0) will always be part of displayed scene.
	// This is to avoid moving a single state and dynamically recalculating
	// rect to center on it... Quickly comes to high position values.
	QRect baseDisplayRectangle(QPoint(-this->displaySize.width()/2, -this->displaySize.height()/2), this->displaySize);


	// Then adjust to include items not seen (includes a margin)
	int leftmostPosition   = baseDisplayRectangle.topLeft().x();
	int topmostPosition    = baseDisplayRectangle.topLeft().y();
	int rightmostPosition  = baseDisplayRectangle.bottomRight().x();
	int bottommostPosition = baseDisplayRectangle.bottomRight().y();

	int margin = 100;

	for (QGraphicsItem* item : this->items())
	{
		// Get item's boundig box scene coordinates
		// Use two points instead of the rect to avoid polygon conversion
		QPointF itemTopLeftF     = item->mapToScene(item->boundingRect().topLeft());
		QPoint  itemTopLeft      = itemTopLeftF.toPoint();
		QPointF itemBottomRightF = item->mapToScene(item->boundingRect().bottomRight());
		QPoint  itemBottomRight  = itemBottomRightF.toPoint();

		if (itemTopLeft.x()-margin < leftmostPosition)
			leftmostPosition = itemTopLeft.x()-margin;

		if (itemTopLeft.y()-margin < topmostPosition)
			topmostPosition = itemTopLeft.y()-margin;

		if (itemBottomRight.x()+margin > rightmostPosition)
			rightmostPosition = itemBottomRight.x()+margin;

		if (itemBottomRight.y()+margin > bottommostPosition)
			bottommostPosition = itemBottomRight.y()+margin;
	}

	QRect finalDisplayRectangle(QPoint(leftmostPosition, topmostPosition), QPoint(rightmostPosition, bottommostPosition));

	this->setSceneRect(finalDisplayRectangle);
}
