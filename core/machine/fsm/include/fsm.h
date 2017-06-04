/*
 * Copyright © 2014-2017 Clément Foucher
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

#ifndef FSM_H
#define FSM_H

// Parent
#include "machine.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class FsmState;
class FsmTransition;
class FsmSimulator;
class FsmUndoCommand;
class FsmGraphicTransition;


class Fsm : public Machine, public enable_shared_from_this<Fsm>
{
    Q_OBJECT

public:
    explicit Fsm();
    ~Fsm();

    // States
    shared_ptr<FsmState> addState(QPointF position, QString name = QString());
    void removeState(shared_ptr<FsmState> state);
    QList<shared_ptr<FsmState> > getStates() const;
    shared_ptr<FsmState> getStateByName(const QString& name) const;
    bool renameState(shared_ptr<FsmState> state, QString newName);

    // Initial state
    void setInitialState(shared_ptr<FsmState> newInitialState);
    shared_ptr<FsmState> getInitialState() const;

    // Transitions
    shared_ptr<FsmTransition> addTransition(shared_ptr<FsmState> source, shared_ptr<FsmState> target, FsmGraphicTransition* representation = nullptr);
    void removeTransition(shared_ptr<FsmTransition> transition);
    QList<shared_ptr<FsmTransition>> getTransitions() const;
    void redirectTransition(shared_ptr<FsmTransition> transition, shared_ptr<FsmState> newSource, shared_ptr<FsmState> newTarget);

    // Simulation
    void setSimulator(shared_ptr<MachineSimulator> simulator) override;
    void forceStateActivation(shared_ptr<FsmState> stateToActivate);

private slots:
    void unmonitoredFsmComponentEditionEventHandler();
    void statePositionChangedEventHandler(shared_ptr<FsmState> originator);
    void transitionSliderPositionChangedEventHandler();

private:
    QString getUniqueStateName(QString nameProposal);

private:
    QList<shared_ptr<FsmState>> states;
    QList<shared_ptr<FsmTransition>> transitions;
    weak_ptr<FsmState> initialState;
};

#endif // FSM_H
