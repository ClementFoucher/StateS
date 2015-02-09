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

#include "logicvalue.h"

class IO;
class Signal;
class Input;
class Output;

class Machine : public QObject
{
    Q_OBJECT

public:
    enum class type{FSM};
    enum class signal_types{Input, Output, LocalVariable, Constant};

public:
    explicit Machine();
    virtual ~Machine();

    virtual Machine::type getType() const = 0;

    QList<Input*>  getInputs() const;
    QList<Output*> getOutputs() const;
    QList<Signal*> getLocalVariables() const;
    QList<Signal*> getConstants() const;

    QList<Signal*> getWrittableSignals() const;
    QList<Signal*> getReadableSignals() const;
    QList<Signal*> getAllVariables() const;
    QList<Signal*> getAllSignals() const;

    virtual void saveMachine(const QString& path) = 0;

public slots:
    bool addSignal(signal_types type, const QString& name);
    bool deleteSignal(const QString& name);
    bool renameSignal(const QString& oldName, const QString& newName);
    bool resizeSignal(const QString& name, uint newSize);
    bool changeSignalInitialValue(const QString& name, LogicValue newValue);

    virtual void simulationModeChanged() = 0;

signals:
    void inputListChangedEvent();
    void outputListChangedEvent();
    void localVariableListChangedEvent();
    void constantListChangedEvent();

protected:
    QHash<QString, Input*>  inputs;
    QHash<QString, Output*> outputs;
    QHash<QString, Signal*> localVariables;
    QHash<QString, Signal*> constants;

private:
    QHash<QString, Signal*> getAllSignalsMap() const;
};

#endif // MACHINE_H
