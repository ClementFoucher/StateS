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
#include "graphictimeline.h"

// Qt classes
#include <QPainter>


GraphicTimeLine::GraphicTimeLine(uint pointsPerCycle, uint eventDelay, bool initialValue, QWidget* parent) :
    QWidget(parent)
{
    this->pointsPerCycle = pointsPerCycle;
    this->eventDelay    = eventDelay;

    this->stepLength = 5;

    this->reset(initialValue);
}

void GraphicTimeLine::addPoint(bool state)
{
    this->points.append(state);

    for (uint i = 0 ; i < this->pointsPerCycle ; i++)
    {
        QPoint lastPosition = timeLinePoly.last();

        if (i < this->eventDelay)
        {
            // Horizontal line with current value before delay
            timeLinePoly.append(QPoint(lastPosition.x() + 1, lastPosition.y()));
        }
        else if (i == this->eventDelay)
        {
            // Edge + horizontal line with new value on delay

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
        }
        else // (i >= this->eventDelay)
        {
            // Horizontal line with new value after delay
            timeLinePoly.append(QPoint(lastPosition.x() + 1, state?1:0));
        }

    }

    this->setMinimumWidth(timeLinePoly.last().x()*this->stepLength + 5*this->stepLength);
    this->setMaximumWidth(timeLinePoly.last().x()*this->stepLength + 5*this->stepLength);

    repaint();
}

void GraphicTimeLine::updateLastPoint(bool state)
{
    // If no change to do, return
    if (this->points.last() == state)
        return;

    // Else change last point
    if (this->points.count() > 1)
    {
        this->removeLastPoint();
        this->addPoint(state);
    }
    else
    {
        // Replace first point
        this->reset(state);
    }
}

void GraphicTimeLine::reset(bool initialValue)
{
    this->points.clear();
    this->points.append(initialValue);

    timeLinePoly.clear();
    // Starting point of graphical vector: not a real point
    timeLinePoly.append(QPoint(0, initialValue?1:0));
    // Actual initial point
    timeLinePoly.append(QPoint(1, initialValue?1:0));

    this->setMinimumWidth(5*this->stepLength);
    this->setMaximumWidth(5*this->stepLength);

    repaint();
}

void GraphicTimeLine::chageEventDelay(uint eventDelay)
{
    this->eventDelay = eventDelay;

    QVector<bool> oldPointVector = this->points;
    this->reset(oldPointVector[0]);

    for (int i = 1 ; i < oldPointVector.count() ; i++)
    {
        this->addPoint(oldPointVector[i]);
    }
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

void GraphicTimeLine::removeLastPoint()
{
    if (this->points.count() > 1) // First point can't be handled: we need an initial value
    {
        for (uint i = 0 ; i < this->pointsPerCycle ; i++)
        {
            this->timeLinePoly.removeLast();

            // Check if there was a edge before, and remove it if so
            if (timeLinePoly[timeLinePoly.count()-1].y() != timeLinePoly[timeLinePoly.count()-2].y())
                timeLinePoly.removeLast();
        }
    }
}

