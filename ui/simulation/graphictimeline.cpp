/*
 * Copyright © 2014 Clément Foucher
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

#include <QPainter>

#include "graphictimeline.h"

GraphicTimeLine::GraphicTimeLine(bool initialState, QWidget* parent) :
    QWidget(parent)
{
    stepLength = 10;

    timeLinePoly.append(QPoint(0, initialState?1:0));
    addPoint(initialState);
}

void GraphicTimeLine::addPoint(bool state)
{

    QPoint lastPosition = timeLinePoly.last();

    if ((lastPosition.y() == 0) && (state == true))
    {
        // Rising edge
        timeLinePoly.append(QPoint(lastPosition.x(), 1));
    }
    else if ((lastPosition.y() == 1) && (state == false))
    {
        // Falling edge
        timeLinePoly.append(QPoint(lastPosition.x(), 0));
    }

    // Horizontal line
    timeLinePoly.append(QPoint(lastPosition.x() + 1, state?1:0));

    this->setMinimumWidth(lastPosition.x()*stepLength + 2*stepLength);
    this->setMaximumWidth(lastPosition.x()*stepLength + 2*stepLength);

    repaint();
}

void GraphicTimeLine::addPointConst()
{
    if (timeLinePoly.last().y() == 1)
        addPoint(true);
    else
        addPoint(false);
}

void GraphicTimeLine::updateLastPoint(bool state)
{
    // If no change to do, return
    if (((timeLinePoly.last().y() == 1) && (state == true)) ||
            ((timeLinePoly.last().y() == 0) && (state == false)))
        return;

    // Else remove last point
    timeLinePoly.removeLast();


    if (timeLinePoly.count() > 2)
    {
        // Check if there was a edge before, and remove it if so
        if (timeLinePoly[timeLinePoly.count()-1].x() == timeLinePoly[timeLinePoly.count()-2].x())
            timeLinePoly.removeLast();
    }
    else
    {
        // Replace first point
        timeLinePoly.removeLast();
        timeLinePoly.append(QPoint(0, state?1:0));
    }


    addPoint(state);
}

void GraphicTimeLine::reset(bool initialState)
{
    timeLinePoly.clear();

    timeLinePoly.append(QPoint(0, initialState?1:0));
    addPoint(initialState);
}


void GraphicTimeLine::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    for (int i = 0 ; i < timeLinePoly.count()-1 ; i++)
    {
        QPoint source;
        QPoint target;

        source.setX(timeLinePoly[i].x()*stepLength + stepLength/2);
        source.setY( (timeLinePoly[i].y() == 1)?5:this->height()-5 );

        target.setX(timeLinePoly[i+1].x()*stepLength + stepLength/2);
        target.setY( (timeLinePoly[i+1].y() == 1)?5:this->height()-5 );

        painter.drawLine(source, target);
    }
}

