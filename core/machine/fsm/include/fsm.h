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


class Fsm : public Machine, public enable_shared_from_this<Fsm>
{
    Q_OBJECT

public:
    explicit Fsm();

    // States
    shared_ptr<FsmState> addState(QString name = QString());
    const QList<shared_ptr<FsmState>>& getStates() const;
    void removeState(shared_ptr<FsmState> state);
    shared_ptr<FsmState> getStateByName(const QString& name) const;

    bool renameState(shared_ptr<FsmState> state, QString newName);

    void setInitialState(const QString &name);
    shared_ptr<FsmState> getInitialState() const;

    // Transitions
    QList<shared_ptr<FsmTransition>> getTransitions() const;

    // Simulation
    void setSimulator(shared_ptr<MachineSimulator> simulator) override;
    void forceStateActivation(shared_ptr<FsmState> stateToActivate);

    // Other
    void clear() override;
    bool isEmpty() const override;

private slots:
    void savableValueEditedEventHandler();

private:
    QString getUniqueStateName(QString nameProposal);

private:
    QList<shared_ptr<FsmState>> states;
    weak_ptr<FsmState> initialState;
    weak_ptr<FsmSimulator> simulator;
};

#endif // FSM_H
