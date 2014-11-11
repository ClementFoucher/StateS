/*
 * Copyright © 2014 Clément Foucher
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

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>

class Machine;
class ResourcesBar;
class SceneWidget;

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(Machine* machine = 0, QWidget* parent = 0);

    void setMachine(Machine* machine);

    SceneWidget* getMachineDisplayArea() const;

protected slots:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void handleSimulationToggled();

private:
    unsigned int verticalSplitterPos;

    // Display
    SceneWidget  * machineDisplayArea = nullptr;
    ResourcesBar * resourcesBar       = nullptr;
};

#endif // CENTRALWIDGET_H
