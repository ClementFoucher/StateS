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


#include <QLabel>
#include <QHBoxLayout>

#include "clocktimeline.h"

#include "graphicclocktimeline.h"

ClockTimeLine::ClockTimeLine(Clock* clock, QWidget* parent) :
    QWidget(parent)
{
    QLabel* title = new QLabel(tr("Clock"));

    GraphicClockTimeLine* timeLineDisplay = new GraphicClockTimeLine(clock);
    timeLineDisplay->setMinimumHeight(title->sizeHint().height());
    timeLineDisplay->setMaximumHeight(title->sizeHint().height());

    QHBoxLayout* layout = new QHBoxLayout(this);

    layout->addWidget(title);
    layout->addWidget(timeLineDisplay);
}
