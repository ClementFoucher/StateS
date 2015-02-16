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

#ifndef SIMULATIONWINDOW_H
#define SIMULATIONWINDOW_H

// Parent
#include <QMainWindow>

// Qt classes
#include <QWidget>
#include <QVBoxLayout>
#include <QToolBar>

// StateS classes
class Machine;
class Clock;

class SimulationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulationWindow(Machine* machine, Clock* clock, QWidget* parent = nullptr);

protected slots:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private slots:
    void exportToPDF();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    Machine* machine = nullptr;

    Clock* clock = nullptr;

    QVBoxLayout* layout = nullptr;

    QToolBar* toolBar = nullptr;

    uint separatorPosition = 0;
};

#endif // SIMULATIONWINDOW_H
