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

// Qt classes
class QDomElement;
class QXmlStreamWriter;

// StateS classes
class FsmState;
class FsmTransition;
class MachineActuatorComponent;
class FsmSimulator;


class Fsm : public Machine
{
    Q_OBJECT

public:
    explicit Fsm();
    //explicit Fsm(const QString& fromPath); // can't be done because need shared_from_this

    // States related
    shared_ptr<FsmState> addState(QString name = QString());
    const QList<shared_ptr<FsmState>>& getStates() const;
    void removeState(shared_ptr<FsmState> state);

    bool renameState(shared_ptr<FsmState> state, QString newName);

    void setInitialState(const QString &name);
    shared_ptr<FsmState> getInitialState() const;

    // Transitions
    QList<shared_ptr<FsmTransition>> getTransitions() const;

    // Save/load
    void loadFromFile(const QString& filePath, bool eraseFirst = false) override;
    void saveMachine(const QString& path) override;

    // Simulation
    void setSimulator(shared_ptr<MachineSimulator> simulator) override;
    void forceStateActivation(shared_ptr<FsmState> stateToActivate);

    // Other
    void clear() override;
    bool isEmpty() const override;

    void exportAsVhdl(const QString& path, bool resetLogicPositive, bool prefixIOs) override;

private:
    QString getUniqueStateName(QString nameProposal);
    shared_ptr<FsmState> getStateByName(const QString& name) const;

    // XML Parser
    void parseXML(const QString& path);
    void parseSignals(QDomElement element);
    void parseStates(QDomElement element);
    void parseTransitions(QDomElement element);
    shared_ptr<Signal> parseEquation(QDomElement element) const;
    void parseActions(QDomElement element, shared_ptr<MachineActuatorComponent> component) const;
    // WML Writer
    void writeLogicEquation(QXmlStreamWriter& stream, shared_ptr<Signal> equation) const;
    void writeActions(QXmlStreamWriter& stream, shared_ptr<MachineActuatorComponent> component) const;

    void stateEditedEventHandler();

private:
    QList<shared_ptr<FsmState>> states;
    weak_ptr<FsmState> initialState;
    weak_ptr<FsmSimulator> simulator;
};

#endif // FSM_H
