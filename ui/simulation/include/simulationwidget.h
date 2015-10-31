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

#ifndef SIMULATIONWIDGET_H
#define SIMULATIONWIDGET_H

// Parent
#include <QMainWindow>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QWidget;
class QToolBar;
class QAction;

// StateS classes
class Machine;
class Clock;
class SimulatorTab;


class SimulationWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulationWidget(shared_ptr<Machine> machine, QWidget* parent = nullptr);

signals:
    void detachTimelineEvent(bool detach);
    void outputDelayChangedEvent(uint newDelay);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private slots:
    void exportToPDF();
    void setMeFree();
    void bindMe();
    void delayOutputOptionTriggered(bool activated);

private:
    QAction* actionDetach = nullptr;

    QToolBar* toolBar = nullptr;

    uint separatorPosition = 0;
};

#endif // SIMULATIONWIDGET_H
