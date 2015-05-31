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

#ifndef GRAPHICTIMELINE_H
#define GRAPHICTIMELINE_H

// Parent
#include <QWidget>

// Qt classes
#include <QPolygon>
#include <QVector>


class GraphicTimeLine : public QWidget
{
    Q_OBJECT

public:
    GraphicTimeLine(uint pointsPerCycle, uint eventDelay, bool initialValue, QWidget* parent = nullptr);

    void addPoint(bool state);
    void updateLastPoint(bool state);
    void reset(bool initialValue);
    void chageEventDelay(uint eventDelay);

protected:
    void paintEvent(QPaintEvent *) override;

    uint stepLength;

private:
    void removeLastPoint();

    QPolygon timeLinePoly;
    QVector<bool> points;
    uint pointsPerCycle;
    uint eventDelay;

};

#endif // GRAPHICTIMELINE_H
