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

#include "variabletimeline.h"

#include "logicvariable.h"
#include "clock.h"
#include "graphictimeline.h"

VariableTimeline::VariableTimeline(LogicVariable* variable, Clock* clock, bool dynamic, QWidget* parent) :
    QWidget(parent)
{
    this->variable = variable;
    this->clock = clock;
    this->isDynamic = dynamic;

    QLabel* varName = new QLabel(variable->getName());

    timeLineDisplay = new GraphicTimeLine(variable->getCurrentState());
    timeLineDisplay->setMinimumHeight(varName->sizeHint().height());
    timeLineDisplay->setMaximumHeight(varName->sizeHint().height());

    this->layout = new QHBoxLayout(this);

    layout->addWidget(varName);
    layout->addWidget(timeLineDisplay);

    if (isDynamic)
        connect(variable, SIGNAL(stateChangedEvent()), this, SLOT(prepareClockEvent()));
    connect(clock, SIGNAL(clockEvent()), this, SLOT(clockEvent()));
    connect(clock, SIGNAL(resetEvent()), this, SLOT(resetEvent()));
}

VariableTimeline::~VariableTimeline()
{
    if (isDynamic)
        disconnect(variable, SIGNAL(stateChangedEvent()), this, SLOT(prepareClockEvent()));
    disconnect(clock, SIGNAL(clockEvent()), this, SLOT(clockEvent()));
    disconnect(clock, SIGNAL(resetEvent()), this, SLOT(resetEvent()));
}

void VariableTimeline::clockEvent()
{
    timeLineDisplay->addPointConst();
    timeLineDisplay->addPoint(variable->getCurrentState());
}

// This is used for input, because state may have changed since previous clock
void VariableTimeline::prepareClockEvent()
{
    timeLineDisplay->updateLastPoint(variable->getCurrentState());
}

void VariableTimeline::resetEvent()
{
    timeLineDisplay->reset(variable->getCurrentState());
}
