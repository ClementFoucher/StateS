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

#ifndef SIMULATORTAB_H
#define SIMULATORTAB_H

// Parent
#include <QWidget>

// Qt classes
class QPushButton;
class QSignalMapper;
class QLineEdit;

// StateS classes
class Fsm;
class FsmState;
class Clock;
class SimulationWindow;
class InputsSelector;

class SimulatorTab : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorTab(Fsm* machine, QWidget* parent = nullptr);
    ~SimulatorTab();

    SimulationWindow* getTimeline() const;

signals:
    void beginSimulation();
    void endSimulation();
    void triggerView();

private slots:
    void triggerSimulationMode(bool enabled);
    void reset();
    void nextStep();
    void clockEvent();
    void resetEvent();
    void targetStateSelectionMade(QObject *choosenTransition);
    void buttonTriggerViewClicked();
    void buttonLauchAutoStepClicked();

private:
    Fsm* machine = nullptr;
    FsmState* currentState = nullptr;

    Clock* clock = nullptr;
    SimulationWindow* timeLine = nullptr;

    QPushButton* buttonTriggerSimulation = nullptr;
    QPushButton* buttonTriggerView = nullptr;
    QPushButton* buttonTriggerAutoStep = nullptr;
    QLineEdit* autoStepValue = nullptr;
    QWidget* simulationTools = nullptr;

    InputsSelector* inputList = nullptr;

    QWidget* targetStateSelection = nullptr;
    QSignalMapper* signalMapper = nullptr;
};

#endif // SIMULATORTAB_H
