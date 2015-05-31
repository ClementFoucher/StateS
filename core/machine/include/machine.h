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

#ifndef MACHINE_H
#define MACHINE_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QList>
#include <QHash>
class QGraphicsItem;

// StateS basic types
#include "logicvalue.h"

// StateS classes
class Signal;
class Input;
class Output;


class Machine : public QObject
{
    Q_OBJECT

public:
    enum class type{FSM};
    enum class signal_type{Input, Output, LocalVariable, Constant};

public:
    explicit Machine();

    virtual Machine::type getType() const = 0;

    QList<shared_ptr<Input>>  getInputs()         const;
    QList<shared_ptr<Output>> getOutputs()        const;
    QList<shared_ptr<Signal>> getLocalVariables() const;
    QList<shared_ptr<Signal>> getConstants()      const;

    QList<shared_ptr<Signal>> getWrittableSignals()        const;
    QList<shared_ptr<Signal>> getReadableSignals()         const;
    QList<shared_ptr<Signal>> getReadableVariableSignals() const;
    QList<shared_ptr<Signal>> getVariablesSignals()        const;
    QList<shared_ptr<Signal>> getIoSignals()               const;
    QList<shared_ptr<Signal>> getAllSignals()              const;

    QList<shared_ptr<Signal>> getInputsAsSignals()  const;
    QList<shared_ptr<Signal>> getOutputsAsSignals() const;

    virtual void loadFromFile(const QString& filePath, bool eraseFirst = false) = 0;
    virtual void saveMachine(const QString& path) = 0;

    virtual void clear();
    virtual bool isEmpty() const;

    QGraphicsItem* getComponentVisualization();

public slots:
    shared_ptr<Signal> addSignal(signal_type type, const QString& name);
    bool deleteSignal(const QString& name);
    bool renameSignal(const QString& oldName, const QString& newName);
    bool resizeSignal(const QString& name, uint newSize);
    bool changeSignalInitialValue(const QString& name, LogicValue newValue);

signals:
    void inputListChangedEvent();
    void outputListChangedEvent();
    void localVariableListChangedEvent();
    void constantListChangedEvent();
    void machineLoadedEvent();

protected:
    QHash<QString, shared_ptr<Input>>  inputs;
    QHash<QString, shared_ptr<Output>> outputs;
    QHash<QString, shared_ptr<Signal>> localVariables;
    QHash<QString, shared_ptr<Signal>> constants;

private:
    QHash<QString, shared_ptr<Signal>> getAllSignalsMap() const;
};

#endif // MACHINE_H
