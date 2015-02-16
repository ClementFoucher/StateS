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

#ifndef FSM_H
#define FSM_H

// Parent
#include "machine.h"

// Qt classes
#include <QDomElement>
#include <QList>
#include <QXmlStreamWriter>

// StateS classes
class FsmState;
class FsmTransition;
class MachineActuatorComponent;

class Fsm : public Machine
{
    Q_OBJECT

public:
    explicit Fsm();
    explicit Fsm(const QString& filePath);
    ~Fsm() override;

    void addState(FsmState* state);
    void removeState(FsmState* state);

    void setInitialState(FsmState* state);
    FsmState* getInitialState() const;

    void addTransition(FsmTransition* transition);
    void removeTransition(FsmTransition* transition);

    Machine::type getType() const override;

    const QList<FsmState *>& getStates() const;

    const QList<FsmTransition *>& getTransitions() const;

    void saveMachine(const QString& path) override;

    // This is an overriden slot
    void simulationModeChanged() override;

private:
    void parseXML(const QString& path);
    void parseSignals(QDomElement element);
    void parseStates(QDomElement element);
    void parseTransitions(QDomElement element);
    Signal* parseEquation(QDomElement element) const;
    void parseActions(QDomElement element, MachineActuatorComponent *component) const;
    void writeLogicEquation(QXmlStreamWriter& stream, Signal* equation) const;
    void writeActions(QXmlStreamWriter& stream, MachineActuatorComponent* component) const;

    QList<FsmState*> states;
    QList<FsmTransition*> transitions;
    FsmState* initialState = nullptr;
};

#endif // FSM_H
