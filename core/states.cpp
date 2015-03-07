/*
 * Copyright © 2014-2015 Clément Foucher
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
#include "states.h"

// Qt classes
#include <QPainter>
#include <QSvgRenderer>

// StateS classes
#include "statesui.h"


QPixmap StateS::getPixmapFromSvg(const QString &path)
{
    QSvgRenderer svgRenderer(path);
    QPixmap pixmap(svgRenderer.defaultSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    svgRenderer.render(&painter);

    return pixmap;
}

StateS::StateS()
{
    drawingWindow = new StatesUi();

    drawingWindow->show();
}

StateS::~StateS()
{
    drawingWindow->setMachine(nullptr);

    delete drawingWindow;
}

QString StateS::getVersion()
{
    return "0.2.6";
}
