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

#ifndef MACHINEBUILDERTAB_H
#define MACHINEBUILDERTAB_H

// Parent
#include <QWidget>

// Qt classes
class QGraphicsView;
class QGraphicsScene;

// To access enums
#include "machine.h"

// StateS classes
class MachineTools;


class MachineBuilderTab : public QWidget
{
    Q_OBJECT

public:
    explicit MachineBuilderTab(Machine::type machineType, shared_ptr<Machine> machine, QWidget* parent = nullptr);

    MachineTools* getBuildTools() const;

    QGraphicsScene* getComponentVisualizationScene();

private slots:
    void updateMachineVisualization();

private:
    weak_ptr<Machine> machine;
    MachineTools* buildTools = nullptr;
    QGraphicsView* componentVisualization = nullptr;
};

#endif // MACHINEBUILDERTAB_H

