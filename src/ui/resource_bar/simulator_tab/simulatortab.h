/*
 * Copyright © 2014-2023 Clément Foucher
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
class QLineEdit;

// StateS classes
class InputsSelector;
class FsmSimulator;
//class CheckBoxHtml;


class SimulatorTab : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatorTab(QWidget* parent = nullptr);
	~SimulatorTab();

	/////
	// Object functions
private slots:
	void triggerSimulationMode(bool enabled);
	void buttonLauchAutoStepClicked();
	void delayOptionToggleEventHandler(bool enabled);

	/////
	// Object variables
private:
	QPushButton  * buttonTriggerSimulation = nullptr;
	QPushButton  * buttonTriggerAutoStep   = nullptr;
//	CheckBoxHtml * checkBoxDelay           = nullptr; // Disabled (buggy)
	QLineEdit    * autoStepValue           = nullptr;
	QWidget      * simulationTools         = nullptr;

	InputsSelector* inputList = nullptr;

};

#endif // SIMULATORTAB_H
