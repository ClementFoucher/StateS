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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QPushButton;
//class QSignalMapper;
class QLineEdit;

// StateS classes
class SimulationWidget;
class InputsSelector;
class FsmSimulator;
class Fsm;
class CheckBoxHtml;


class SimulatorTab : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorTab(shared_ptr<Fsm> machine, QWidget* parent = nullptr);
    ~SimulatorTab();

signals:
    void beginSimulationEvent();
    void endSimulationEvent();

private slots:
    void triggerSimulationMode(bool enabled);
    void buttonLauchAutoStepClicked();
    void delayOptionToggleEventHandler(bool enabled);

private:
    weak_ptr<Fsm> machine;
    shared_ptr<FsmSimulator> simulator;

    SimulationWidget* timeLine = nullptr;

    // QWidgets with parent
    QPushButton  * buttonTriggerSimulation = nullptr;
    QPushButton  * buttonTriggerAutoStep   = nullptr;
    CheckBoxHtml * checkBoxDelay           = nullptr;
    QLineEdit    * autoStepValue           = nullptr;
    QWidget      * simulationTools         = nullptr;

    InputsSelector* inputList = nullptr;
};

#endif // SIMULATORTAB_H
