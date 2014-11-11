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

#ifndef SIMULATORTAB_H
#define SIMULATORTAB_H

#include <QWidget>

#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>

class Fsm;
class FsmState;
class Clock;
class SimulationTimeLine;

class SimulatorTab : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorTab(Fsm* machine, QWidget* parent = 0);
    ~SimulatorTab();

signals:
    void beginSimulation();
    void endSimulation();

private slots:
    void triggerSimulationMode(bool enabled);
    void reset();
    void nextStep();
    void updateInputs();
    void clockEvent();
    void resetEvent();

private:
    Fsm* machine = nullptr;
    FsmState* currentState = nullptr;

    Clock* clock = nullptr;

    SimulationTimeLine* timeLine = nullptr;

    QGridLayout* layout = nullptr;

    QPushButton * buttonTriggerSimulation = nullptr;
    QPushButton * buttonReset = nullptr;
    QPushButton * buttonNextStep = nullptr;

    QListWidget* inputList = nullptr;
};

#endif // SIMULATORTAB_H
