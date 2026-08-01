/*
 * Copyright © 2014-2026 Clément Foucher
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

// Qt
#include <QGraphicsView>


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

	auto newSceneRect = this->getItemsBoundingRect(100);

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
