/*
 * Copyright © 2025-2026 Clément Foucher
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

#ifndef SIMULATEDACTIONONVARIABLE_H
#define SIMULATEDACTIONONVARIABLE_H

// Stdlib
#include <memory>

// Qt
#include <QCoreApplication>

// StateS
#include "actiononvariable.h"
class SimulatedEquation;


class SimulatedActionOnVariable
{
	Q_DECLARE_TR_FUNCTIONS(SimulatedActionOnVariable)

	/////
	// Constructors/destructors
public:
	explicit SimulatedActionOnVariable(std::shared_ptr<const ActionOnVariable> sourceAction); // Throws StatesException

	/////
	// Object functions
public:
	void doAction();

	bool isActionMemorized() const;

	ComponentId getVariableId() const;

	/////
	// Object variables
private:
	ComponentId                        targetVariableId = nullId;
	ActionOnVariable::Behavior_t       behavior         = ActionOnVariable::Behavior_t::none;
	std::shared_ptr<SimulatedEquation> valueProvider;
	std::shared_ptr<SimulatedEquation> condition;
	int                                rangeL           = -1;
	int                                rangeR           = -1;

};

#endif // SIMULATEDACTIONONVARIABLE_H
