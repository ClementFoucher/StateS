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

#ifndef SIGNALTIMELINE_H
#define SIGNALTIMELINE_H

#include <QWidget>

#include <QList>

class Signal;
class Clock;
class GraphicTimeLine;

class SignalTimeline : public QWidget
{
    Q_OBJECT

public:
    explicit SignalTimeline(Signal* variable, Clock* clock, bool dynamic = false, QWidget* parent = 0);

private slots:
    void clockEvent();
    void prepareClockEvent();
    void resetEvent();

private:
    bool isDynamic;
    Signal* variable  = nullptr;
    Clock* clock  = nullptr;

    QList<GraphicTimeLine*> variableLineDisplay;
};

#endif // SIGNALTIMELINE_H
