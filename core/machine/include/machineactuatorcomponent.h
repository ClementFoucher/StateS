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

#ifndef MACHINEACTUATORCOMPONENT_H
#define MACHINEACTUATORCOMPONENT_H

#include "machinecomponent.h"

#include <QMap>

#include "logicvalue.h"

class Machine;
class Signal;

class MachineActuatorComponent : public MachineComponent
{
    Q_OBJECT

public:
    enum class action_types { pulse, set, reset, assign };

public:
    explicit MachineActuatorComponent(Machine* owningMachine);

    QList<Signal*> getActions() const;
    void setActions(const QList<Signal *> &newActions);
    void clearActions();

    void addActionByName(const QString& signalName);
    void addAction(Signal* signal);
    bool removeActionByName(const QString& signalName);

    void activateActions();

    void setActionType(Signal* signal, action_types type);
    bool setActionValue(Signal* signal, LogicValue value);

    action_types getActionType(Signal *variable);
    LogicValue getActionValue(Signal *variable);

public slots:
    void removeAction(Signal* signal);

protected:
    QList<Signal*> actions;

    QMap<Signal*, action_types> actionType;
    QMap<Signal*, LogicValue> actionValue;

private slots:
    void signalResizedEventHandler();
};

#endif // MACHINEACTUATORCOMPONENT_H
