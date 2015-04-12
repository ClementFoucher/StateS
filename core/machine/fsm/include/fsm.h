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
#include <QList>
class QDomElement;
class QXmlStreamWriter;
class QString;

// StateS classes
class FsmState;
class FsmTransition;
class MachineActuatorComponent;


class Fsm : public Machine, public enable_shared_from_this<Fsm>
{
    Q_OBJECT

public:
    explicit Fsm();

    void loadFromFile(const QString& filePath, bool eraseFirst = false) override;

    shared_ptr<FsmState> addState(QString name = QString());
    void removeState(shared_ptr<FsmState> state);
    bool renameState(shared_ptr<FsmState> state, QString newName);

    QList<shared_ptr<FsmTransition>> getTransitions() const;

    void setInitialState(const QString &name);
    shared_ptr<FsmState> getInitialState() const;

    Machine::type getType() const override;

    const QList<shared_ptr<FsmState>>& getStates() const;

    void saveMachine(const QString& path) override;

    void clear() override;

    // This is an overriden slot
    void simulationModeChanged() override;

    bool isEmpty() const override;

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


    QList<shared_ptr<FsmState>> states;
    weak_ptr<FsmState> initialState;
};

#endif // FSM_H
