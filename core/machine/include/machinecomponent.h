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

#ifndef MACHINECOMPONENT_H
#define MACHINECOMPONENT_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Machine;


class MachineComponent : public QObject
{
    Q_OBJECT

public:
    explicit MachineComponent(shared_ptr<Machine> owningMachine);

    shared_ptr<Machine> getOwningMachine() const;

signals:
    void componentStaticConfigurationChangedEvent(); // Triggered when object "savable" values are modified
    void componentDynamicStateChangedEvent();        // Triggered when object "discardable" values are modified

private:
    weak_ptr<Machine> owningMachine;
};

#endif // MACHINECOMPONENT_H
