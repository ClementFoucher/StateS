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
#include "genericscene.h"

// Qt classes
#include <QGraphicsItem>
#include <QGraphicsView>

// StateS classes
#include "machinemanager.h"


GenericScene::GenericScene()
{
	connect(machineManager.get(), &MachineManager::simulationModeChangedEvent, this, &GenericScene::simulationModeChangeEventHandler);
}

QRectF GenericScene::getItemsBoundingRect()
{
	if (this->items().count() == 0) return QRectF();

	// Use one item to initialize rect
	auto firstItem = this->items().at(0);
	qreal leftmostPosition   = firstItem->mapToScene(firstItem->boundingRect().topLeft()).x();
	qreal topmostPosition    = firstItem->mapToScene(firstItem->boundingRect().topLeft()).y();
	qreal rightmostPosition  = firstItem->mapToScene(firstItem->boundingRect().bottomRight()).x();
	qreal bottommostPosition = firstItem->mapToScene(firstItem->boundingRect().bottomRight()).y();

	// Then adjust to include all items
	const auto items = this->items();
	for (QGraphicsItem* item : items)
	{
		// Get item's boundig box scene coordinates
		// Use two points instead of the rect to avoid polygon conversion
		QPointF itemTopLeft     = item->mapToScene(item->boundingRect().topLeft());
		QPointF itemBottomRight = item->mapToScene(item->boundingRect().bottomRight());

		if (itemTopLeft.x() < leftmostPosition)
			leftmostPosition = itemTopLeft.x();

		if (itemTopLeft.y() < topmostPosition)
			topmostPosition = itemTopLeft.y();

		if (itemBottomRight.x() > rightmostPosition)
			rightmostPosition = itemBottomRight.x();

		if (itemBottomRight.y() > bottommostPosition)
			bottommostPosition = itemBottomRight.y();
	}

	// Build rect with a margin
	const int margin = 100;
	QRectF finalDisplayRectangle(QPoint(leftmostPosition-margin, topmostPosition-margin), QPoint(rightmostPosition+margin, bottommostPosition+margin));

	return finalDisplayRectangle;
}

void GenericScene::recomputeSceneRect()
{
	// Ensure the scene rect always covers at least
	// the size of the view vidget, centered on (0, 0)

	if (this->views().count() == 0) return;

	auto displaySize = this->views().at(0)->size();
	auto left   = -displaySize.width()/2;
	auto right  =  displaySize.width()/2;
	auto top    = -displaySize.height()/2;
	auto bottom =  displaySize.height()/2;

	auto newSceneRect = this->getItemsBoundingRect();

	if (newSceneRect.left() > left)
	{
		newSceneRect.setLeft(left);
	}
	if (newSceneRect.right() < right)
	{
		newSceneRect.setRight(right);
	}
	if (newSceneRect.top() > top)
	{
		newSceneRect.setTop(top);
	}
	if (newSceneRect.bottom() < bottom)
	{
		newSceneRect.setBottom(bottom);
	}

	this->setSceneRect(newSceneRect);
}

void GenericScene::simulationModeChangeEventHandler(SimulationMode_t newMode)
{
	emit this->sceneSimulationModeAboutToChangeEvent();
	this->updateSimulatioMode(newMode);
	emit this->sceneSimulationModeChangedEvent();
}
