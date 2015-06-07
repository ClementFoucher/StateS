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

#ifndef FSMSIMULATOR_H
#define FSMSIMULATOR_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>
#include <QWidget>
class QSignalMapper;

// StateS classes
class Clock;
class Fsm;
class FsmState;
class FsmTransition;


class FsmSimulator : public QObject
{
    Q_OBJECT

public:
    explicit FsmSimulator(shared_ptr<Fsm> machine);

    shared_ptr<Clock> getClock() const;

    void reset();
    void doStep();
    void start(uint period);
    void suspend();
    void targetStateSelectionMadeEventHandler(int i);
    void forceStateActivation(shared_ptr<FsmState> stateToActivate);

private slots:
    void clockEventHandler();
    void resetEventHandler();

private:
    void activateTransition(shared_ptr<FsmTransition> transition);

    shared_ptr<Clock> clock;
    weak_ptr<Fsm> machine;

    weak_ptr<FsmState> currentState;
    weak_ptr<FsmTransition> latestTransitionCrossed;
    QMap<uint, shared_ptr<FsmTransition>> potentialTransitions;

    unique_ptr<QWidget> targetStateSelector;
    QSignalMapper* signalMapper  = nullptr; // Use pointer because we need a deleteLater instruction

};

#endif // FSMSIMULATOR_H
