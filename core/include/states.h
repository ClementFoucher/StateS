/*
 * Copyright © 2014-2017 Clément Foucher
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

#ifndef STATES_H
#define STATES_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QStack>
#include <QList>

// Diff Match Patch classes
#include "diff_match_patch.h"

// StateS classes
class StatesUi;
class Machine;
class MachineConfiguration;


/**
 * @brief The StateS class is the root object of this application:
 * it owns the UI and the current machine under edition.
 *
 * It is the only one able to change the current machine.
 *
 * This class is also used statically for generic functions.
 */
class StateS : public QObject
{
    Q_OBJECT

public:
    static QString getVersion();

public:
    explicit StateS(const QString& initialFilePath = QString::null);

    void run();

public slots:
    void addCheckpoint();
    void machineChanged();
    void undo();

private slots:
    void clearMachine();

    void saveCurrentMachine(const QString& path);
    void saveCurrentMachineInCurrentFile();

    void generateNewFsm();
    void loadFsm(const QString& path);

private:
    void refreshMachine(shared_ptr<Machine> newMachine);
    void loadNewMachine(shared_ptr<Machine> newMachine, const QString& path = QString::null);
    void updateLatestXml();
    void updateFilePath(const QString& newPath);

private:
    shared_ptr<StatesUi> statesUi;
    shared_ptr<Machine>  machine;

    QString currentFilePath = QString::null;

    QString latestXmlCode;
    QStack<QList<Patch>> undoQueue;

    bool machineIsAtCheckpoint;
    bool machineSaved;
};

#endif // STATES_H
