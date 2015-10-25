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
class MachineBuilder;
class Signal;
class Input;
class Output;


class Machine : public QObject
{
    Q_OBJECT

public:
    enum class signal_type{Input, Output, LocalVariable, Constant};
    enum class mode{voidMode, editMode, simulateMode};

public:
    explicit Machine();

    QString getName() const;

    QList<shared_ptr<Input>>  getInputs()         const;
    QList<shared_ptr<Output>> getOutputs()        const;
    QList<shared_ptr<Signal>> getLocalVariables() const;
    QList<shared_ptr<Signal>> getConstants()      const;

    QList<shared_ptr<Signal>> getInputsAsSignals()  const;
    QList<shared_ptr<Signal>> getOutputsAsSignals() const;

    QList<shared_ptr<Signal>> getWrittableSignals()        const;
    QList<shared_ptr<Signal>> getReadableSignals()         const;
    QList<shared_ptr<Signal>> getReadableVariableSignals() const;
    QList<shared_ptr<Signal>> getVariablesSignals()        const;
    QList<shared_ptr<Signal>> getIoSignals()               const;
    QList<shared_ptr<Signal>> getAllSignals()              const;

    bool cleanSignalName(QString& nameToClean) const;
    QString getUniqueSignalName(const QString& prefix) const;

    virtual void loadFromFile(const QString& filePath, bool eraseFirst = false) = 0;
    virtual void saveMachine(const QString& path) = 0;

    virtual void clear();
    virtual bool isEmpty() const;

    void setMode(mode newMode);
    mode getCurrentMode();

    shared_ptr<MachineBuilder> getMachineBuilder() const;

    QGraphicsItem* getComponentVisualization();

public slots:
    bool setName(const QString& newName);
    shared_ptr<Signal> addSignal(signal_type type, const QString& name);
    bool deleteSignal(const QString& name);
    bool renameSignal(const QString& oldName, const QString& newName);
    bool resizeSignal(const QString& name, uint newSize);
    bool changeSignalInitialValue(const QString& name, LogicValue newValue);
    bool changeSignalRank(const QString& name, uint newRank);

signals:
    void nameChangedEvent(const QString& newName);
    void changedModeEvent(mode newMode);
    void inputListChangedEvent();
    void outputListChangedEvent();
    void localVariableListChangedEvent();
    void constantListChangedEvent();
    void machineLoadedEvent();
    void componentVisualizationUpdatedEvent();

protected:
    // Store all signals as shared_ptr<Signal> for helper functions,
    // but input/output lists are actually shared_ptr<Input/Output>

    // Mutex required for list edition?
    QHash<QString, shared_ptr<Signal>> inputs;
    QHash<QString, shared_ptr<Signal>> outputs;
    QHash<QString, shared_ptr<Signal>> localVariables;
    QHash<QString, shared_ptr<Signal>> constants;

    QHash<QString, uint> inputsRanks;
    QHash<QString, uint> outputsRanks;
    QHash<QString, uint> localVariablesRanks;
    QHash<QString, uint> constantsRanks;

    QString name;

private:

    shared_ptr<Signal> addSignalAtRank(signal_type type, const QString& name, uint rank);
    QList<shared_ptr<Signal>> getRankedSignalList(const QHash<QString, shared_ptr<Signal>>* signalHash, const QHash<QString, uint>* rankHash) const;
    void addSignalToList(shared_ptr<Signal> signal, uint rank, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);
    bool deleteSignalFromList(const QString& name, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);
    bool renameSignalInList(const QString& oldName, const QString& newName, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);
    bool changeRankInList(const QString& name, uint newRank, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);

    void rebuildComponentVisualization();
    QHash<QString, shared_ptr<Signal>> getAllSignalsMap() const;

    // Local copy of visu => pointer because scene takes ownership
    QGraphicsItem* componentVisu = nullptr;

    bool inhibateEvent = false;
    shared_ptr<MachineBuilder> machineBuilder = nullptr;
    mode currentMode = mode::editMode;
};

#endif // MACHINE_H
