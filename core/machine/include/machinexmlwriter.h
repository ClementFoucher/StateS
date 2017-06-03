/*
 * Copyright © 2017 Clément Foucher
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
#ifndef MACHINEXMLWRITER_H
#define MACHINEXMLWRITER_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QXmlStreamWriter;
class QFile;

// StateS classes
class Machine;
class MachineConfiguration;
class Signal;
class MachineActuatorComponent;


class MachineXmlWriter : public QObject
{
    Q_OBJECT

public: // Static

    enum MachineaveFileManagerErrorEnum
    {
        unable_to_replace = 0,
        unkown_directory  = 1,
        unable_to_open    = 2
    };

public:
    explicit MachineXmlWriter(QObject* parent = nullptr);

    virtual void writeMachineToFile(shared_ptr<Machine> machine, shared_ptr<MachineConfiguration> configuration, const QString& filePath) = 0; // Throws StatesException
    virtual QString getMachineXml(shared_ptr<Machine> machine) = 0;

protected:
    void createSaveFile(const QString& filePath); // Throws StatesException
    void createSaveString();
    void finalizeSaveFile();

    void writeMachineCommonElements(shared_ptr<Machine> machine);
    void writeMachineConfiguration();
    void writeMachineSignals(shared_ptr<Machine> machine);
    void writeActuatorActions(shared_ptr<MachineActuatorComponent> component);
    void writeLogicEquation(shared_ptr<Signal> equation);

protected:
    shared_ptr<QXmlStreamWriter> stream;
    QString xmlString;
    shared_ptr<MachineConfiguration> configuration;

private:
    shared_ptr<QFile> file;
};

#endif // MACHINEXMLWRITER_H
