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

#ifndef MACHINEACTUATORELEMENT_H
#define MACHINEACTUATORELEMENT_H

#include "machineelement.h"

#include <QMap>

#include "logicvariable.h"

class Machine;

class MachineActuatorElement : public MachineElement
{
    Q_OBJECT

public:
    explicit MachineActuatorElement(Machine* owningMachine);

    QList<LogicVariable*> getActions() const;
    void setActions(const QList<LogicVariable*>* newActions);
    void clearActions();

    void addAction(const QString& variableName);
    void addAction(LogicVariable* variable);
    void removeAction(const QString& variableName);

    void setActionType(LogicVariable *variable, LogicVariable::action_types type);
    LogicVariable::action_types getActionType(LogicVariable *variable);

public slots:
    void removeAction(LogicVariable* variable);

signals:
    void elementConfigurationChanged();

protected:
    QList<LogicVariable*> actions;

    QMap<LogicVariable*, LogicVariable::action_types> actionType;
};

#endif // MACHINEACTUATORELEMENT_H
