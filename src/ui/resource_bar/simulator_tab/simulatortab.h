/*
 * Copyright © 2014-2025 Clément Foucher
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

// Qt
class QPushButton;
class QVBoxLayout;
class QGroupBox;

// StateS
class SimulatorConfigurator;


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

	/////
	// Object variables
private:
	QVBoxLayout* boxLayout = nullptr;

	QPushButton* buttonTriggerSimulation = nullptr;
	SimulatorConfigurator* simulatorConfigurator = nullptr;

	QGroupBox* configurationGroup = nullptr;
	QGroupBox* timeManagerGroup   = nullptr;
	QGroupBox* inputsGroup        = nullptr;

};

#endif // SIMULATORTAB_H
