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
#ifndef FSMSAVEFILEMANAGER_H
#define FSMSAVEFILEMANAGER_H

// Parent
#include "machinesavefilemanager.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QString;
class QDomElement;

// StateS classes
class Fsm;
class MachineConfiguration;


class FsmSaveFileManager : public MachineSaveFileManager
{
    Q_OBJECT

public: // Static

    enum FsmSaveFileManagerErrorEnum
    {
        wrong_xml = 0
    };

public:
    explicit FsmSaveFileManager(QObject* parent = nullptr);

    void writeMachineToFile(shared_ptr<Machine> machine, shared_ptr<MachineConfiguration> configuration, const QString& filePath) override; // Throws StatesException
    shared_ptr<Machine> loadMachineFromFile(const QString& filePath) override; // Throws StatesException

    QString getMachineXml(shared_ptr<Machine> machine) override;
    shared_ptr<Machine> loadMachineFromXml(const QString& machineXml) override;

private:
    void writeFsmToStream(shared_ptr<Fsm> machine, shared_ptr<MachineConfiguration> configuration);

    void writeFsmStates(shared_ptr<Fsm> machine, shared_ptr<MachineConfiguration> configuration);
    void writeFsmTransitions(shared_ptr<Fsm> machine);

    shared_ptr<Fsm> loadFsmFromDocument();

    void parseFsmStates(QDomElement element, shared_ptr<Fsm> machine);
    void parseFsmTransitions(QDomElement element, shared_ptr<Fsm> machine);
};

#endif // FSMSAVEFILEMANAGER_H
