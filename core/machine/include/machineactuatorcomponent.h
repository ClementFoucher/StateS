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

#ifndef MACHINEACTUATORCOMPONENT_H
#define MACHINEACTUATORCOMPONENT_H

// Parent
#include "machinecomponent.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>
#include <QList>
class QString;

// StateS basic types
#include "logicvalue.h"

// StateS classes
class Machine;
class Signal;


class MachineActuatorComponent : public MachineComponent
{
    Q_OBJECT

public:
    enum class action_types { activeOnState, pulse, set, reset, assign };
    typedef enum { none = 0x0, activeOnState = 0x1, pulse = 0x2, set = 0x4, reset = 0x8, assign = 0x10 } allowed_action_types;

public:
    explicit MachineActuatorComponent(shared_ptr<Machine> owningMachine);

    QList<shared_ptr<Signal>> getActions();
    void setActions(const QList<shared_ptr<Signal>>& newActions);
    void clearActions();

    void addActionByName(const QString& signalName);
    void addAction(shared_ptr<Signal> signal);
    bool removeActionByName(const QString& signalName);

    void activateActions();

    void setActionType(shared_ptr<Signal> signal, action_types type);
    bool setActionValue(shared_ptr<Signal> signal, LogicValue value);

    action_types getActionType(shared_ptr<Signal> variable);
    LogicValue getActionValue(shared_ptr<Signal> variable);

    uint getAllowedActionTypes() const;


signals:
    void actionListChangedEvent();

protected:
    void setAllowedActionTypes(uint flags);

    QList<weak_ptr<Signal>> actions;

    // Reference signals by name as we can assert they are unique
    QMap<QString, action_types> actionType;
    QMap<QString, LogicValue>   actionValue;

private slots:
    void removeAction(shared_ptr<Signal> signal);
    void signalResizedEventHandler();
    void cleanActionList();

private:
    uint allowedActionTypes = none;
};

#endif // MACHINEACTUATORCOMPONENT_H
