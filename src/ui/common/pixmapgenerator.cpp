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
#include "pixmapgenerator.h"

// Qt
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>

// StateS
#include "graphicfsmstate.h"
#include "graphicfsmtransition.h"


QPixmap PixmapGenerator::getStatesLogo(int size, double angle)
{
	QSvgRenderer svgRenderer(QString(":/icons/StateS"));
	QPixmap pixmap(size, size);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	svgRenderer.render(&painter);

	if (angle != 0)
	{
		QTransform rotation;
		rotation.rotate(angle);
		return pixmap.transformed(rotation);
	}
	else
	{
		return pixmap;
	}
}

QPixmap PixmapGenerator::getFsmStateCursor()
{
	return GraphicFsmState::getPixmap(32, false, true);
}

QPixmap PixmapGenerator::getFsmTransitionCursor()
{
	return GraphicFsmTransition::getPixmap(32);
}
