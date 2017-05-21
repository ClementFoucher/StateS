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
#ifndef MACHINESAVEFILEMANAGER_H
#define MACHINESAVEFILEMANAGER_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QXmlStreamWriter;
class QDomElement;
class QFile;
class QDomDocument;

// StateS classes
class Machine;
class MachineConfiguration;
class Signal;
class MachineActuatorComponent;


class MachineSaveFileManager : public QObject
{
    Q_OBJECT

public: // Static

    enum MachineaveFileManagerErrorEnum
    {
        unable_to_replace = 0,
        unkown_directory  = 1,
        unable_to_open    = 2,
        inexistant_file   = 3,
        permission_denied = 4,
        wrong_xml         = 5
    };

public:
    explicit MachineSaveFileManager(QObject* parent = nullptr);

    virtual void writeMachineToFile(shared_ptr<Machine> machine, shared_ptr<MachineConfiguration> configuration, const QString& filePath) = 0; // Throws StatesException
    virtual shared_ptr<Machine> loadMachineFromFile(const QString& filePath) = 0; // Throws StatesException

    virtual QString getMachineXml(shared_ptr<Machine> machine) = 0;
    virtual shared_ptr<Machine> loadMachineFromXml(const QString& machineXml) = 0;

    shared_ptr<MachineConfiguration> getConfiguration();
    QList<QString> getWarnings();

protected:
    // Saving
    void createSaveFile(const QString& filePath); // Throws StatesException
    void createSaveString();
    void finalizeSaveFile();

    void writeMachineCommonElements(shared_ptr<Machine> machine, shared_ptr<MachineConfiguration> configuration);
    void writeMachineConfiguration(shared_ptr<MachineConfiguration> configuration);
    void writeMachineSignals(shared_ptr<Machine> machine);
    void writeActuatorActions(shared_ptr<MachineActuatorComponent> component);
    void writeLogicEquation(shared_ptr<Signal> equation);

    // Loading
    void initializeDocumentFromString(const QString& machineXml); // Throws StatesException
    void initializeDocumentFromFile(const QString& filePath); // Throws StatesException
    void closeLoadFile();

    void parseMachineName(QDomElement rootNode, shared_ptr<Machine> machine);
    void parseMachineConfiguration(QDomElement element);
    void parseMachineSignals(QDomElement element, shared_ptr<Machine> machine);
    void parseActuatorActions(QDomElement element, shared_ptr<MachineActuatorComponent> component, shared_ptr<Machine> machine);
    shared_ptr<Signal> parseEquation(QDomElement element, shared_ptr<Machine> machine);

protected:
    QList<QString> warnings;
    shared_ptr<QXmlStreamWriter> stream;
    shared_ptr<QDomDocument> document;
    QString xmlString;

private:
    shared_ptr<MachineConfiguration> configuration;
    shared_ptr<QFile> file;
};

#endif // MACHINESAVEFILEMANAGER_H
