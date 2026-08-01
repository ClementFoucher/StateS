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

// Current class header
#include "statesscene.h"

// Qt
#include <QGraphicsItem>


QRectF StatesScene::getItemsBoundingRect(uint margin) const
{
	const auto items = this->items();
	if (items.count() == 0) return QRectF();


	// Use one item to initialize rect
	auto firstItem = items.at(0);

	QPointF itemTopLeft     = firstItem->mapToScene(firstItem->boundingRect().topLeft());
	QPointF itemBottomRight = firstItem->mapToScene(firstItem->boundingRect().bottomRight());

	qreal leftestPosition  = itemTopLeft.x();
	qreal highestPosition  = itemTopLeft.y();
	qreal rightestPosition = itemBottomRight.x();
	qreal lowestPosition   = itemBottomRight.y();

	// Then adjust to include all items
	for (auto item : items)
	{
		// Get item's boundig box scene coordinates
		// Use two points instead of the rect to avoid polygon conversion
		itemTopLeft     = item->mapToScene(item->boundingRect().topLeft());
		itemBottomRight = item->mapToScene(item->boundingRect().bottomRight());

		if (itemTopLeft.x() < leftestPosition)
		{
			leftestPosition = itemTopLeft.x();
		}

		if (itemTopLeft.y() < highestPosition)
		{
			highestPosition = itemTopLeft.y();
		}

		if (itemBottomRight.x() > rightestPosition)
		{
			rightestPosition = itemBottomRight.x();
		}

		if (itemBottomRight.y() > lowestPosition)
		{
			lowestPosition = itemBottomRight.y();
		}
	}

	// Build rect with a margin
	QRectF finalDisplayRectangle(QPointF(leftestPosition-margin, highestPosition-margin), QPointF(rightestPosition+margin, lowestPosition+margin));
	return finalDisplayRectangle;
}

QGraphicsTextItem* StatesScene::buildGraphicsTextItem(const QString& text) const
{
	QString htmlText = "<span style=\"color:black;\">" + text + "</span>";
	auto textItem = new QGraphicsTextItem();
	textItem->setHtml(htmlText);

	return textItem;
}
