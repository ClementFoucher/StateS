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
#ifndef FSMSAVEFILEMANAGER_H
#define FSMSAVEFILEMANAGER_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QString;
class QXmlStreamWriter;
class QDomElement;

// StateS classes
class Fsm;
class Signal;
class MachineActuatorComponent;


class FsmSaveFileManager : public QObject
{
    Q_OBJECT

public: // Static

    enum FsmSaveFileManagerErrorEnum
    {
        unable_to_replace = 0,
        permission_denied = 1,
        unkown_directory  = 2,
        unable_to_open    = 3,
        inexistant_file   = 4,
        wrong_xml         = 5
    };

public:
    static void writeToFile(shared_ptr<Fsm> machine, const QString& filePath); // Throws StatesException
    static shared_ptr<Fsm> loadFromFile(const QString& filePath); // Throws StatesException
    static QList<QString> getLastOperationWarnings();

private:
    static void writeSignals(QXmlStreamWriter& stream, shared_ptr<Fsm> machine);
    static void writeStates(QXmlStreamWriter& stream, shared_ptr<Fsm> machine);
    static void writeTransitions(QXmlStreamWriter& stream, shared_ptr<Fsm> machine);
    static void writeLogicEquation(QXmlStreamWriter& stream, shared_ptr<Signal> equation);
    static void writeActions(QXmlStreamWriter& stream, shared_ptr<MachineActuatorComponent> component);

    static void parseSignals(QDomElement element, shared_ptr<Fsm> machine);
    static void parseStates(QDomElement element, shared_ptr<Fsm> machine);
    static void parseTransitions(QDomElement element, shared_ptr<Fsm> machine);
    static shared_ptr<Signal> parseEquation(QDomElement element, shared_ptr<Fsm> machine);
    static void parseActions(QDomElement element, shared_ptr<MachineActuatorComponent> component, shared_ptr<Fsm> machine);

private:
    static QList<QString> warnings;
};

#endif // FSMSAVEFILEMANAGER_H
