/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIMULATORCONFIGURATOR_H
#define SIMULATORCONFIGURATOR_H

// Parent
#include <QWidget>

// Qt classes
class QComboBox;

// StateS classes
#include <statestypes.h>


class SimulatorConfigurator : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatorConfigurator(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	SimulationBehavior_t getMemorizedStateActionBehavior()      const;
	SimulationBehavior_t getContinuousStateActionBehavior()     const;
	SimulationBehavior_t getMemorizedTransitionActionBehavior() const;
	SimulationBehavior_t getPulseTransitionActionBehavior()     const;

	/////
	// Object variables
private:
	QComboBox* memorizedState      = nullptr;
	QComboBox* continuousState     = nullptr;
	QComboBox* memorizedTransition = nullptr;
	QComboBox* pulseTransition     = nullptr;

};

#endif // SIMULATORCONFIGURATOR_H
