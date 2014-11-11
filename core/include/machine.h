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

#ifndef MACHINE_H
#define MACHINE_H

#include <QObject>

#include <QList>
#include <QHash>

class IO;
class LogicVariable;
class Input;
class Output;

class Machine : public QObject
{
    Q_OBJECT

public:
    enum class type{FSM};

public:
    explicit Machine();
    virtual ~Machine();

    virtual Machine::type getType() const = 0;

    QList<Input*> getInputs() const;
    QList<Output*> getOutputs() const;
    QList<LogicVariable*> getLocalVariables() const;

    QList<LogicVariable*> getWrittableVariables() const;
    QList<LogicVariable*> getReadableVariables() const;
    QList<LogicVariable*> getAllVariables() const;

    virtual void saveMachine(const QString& path) = 0;

public slots:
    bool addInput(const QString& name);
    bool addOutput(const QString& name);
    bool addLocalVariable(const QString& name);

    bool deleteInput(const QString& name);
    bool deleteOutput(const QString& name);
    bool deleteLocalVariable(const QString& name);

    bool renameInput(const QString& oldName, const QString& newName);
    bool renameOutput(const QString& oldName, const QString& newName);
    bool renameLocalVariable(const QString& oldName, const QString& newName);

    virtual void simulationModeChanged() = 0;

signals:
    void InputListChangedEvent();
    void OutputListChangedEvent();
    void LocalVariableListChangedEvent();

protected:
    bool addVariable(QHash< QString, LogicVariable*>* list, LogicVariable* newVariable);
    bool renameVariable(QHash< QString, LogicVariable*>* list, const QString& oldName, const QString& newName);

    QHash< QString, Input*>         inputs;
    QHash< QString, Output*>        outputs;
    QHash< QString, LogicVariable*> localVariables;
};

#endif // MACHINE_H
