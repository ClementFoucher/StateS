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

// StateS classes
class Machine;
class MachineConfiguration;


class MachineSaveFileManager : public QObject
{
    Q_OBJECT

public:
    explicit MachineSaveFileManager(QObject* parent = nullptr);

    shared_ptr<MachineConfiguration> getConfiguration();

protected:
    void writeConfiguration(QXmlStreamWriter& stream, shared_ptr<MachineConfiguration> configuration);
    void writeSignals(QXmlStreamWriter& stream, shared_ptr<Machine> machine);

    void parseConfiguration(QDomElement element);
    void parseSignals(QDomElement element, shared_ptr<Machine> machine);

protected:
    QList<QString> warnings;

private:
    shared_ptr<MachineConfiguration> configuration;
};

#endif // MACHINESAVEFILEMANAGER_H
