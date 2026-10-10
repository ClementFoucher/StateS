/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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

#ifndef EQUATIONCOMPUTER_H
#define EQUATIONCOMPUTER_H

// StateS
#include "machinevalue.h"
#include "equation.h"


class EquationComputer
{
	/////
	// Static functions
public:
	static MachineValue computeValue(Equation::Operator_t operator_, const std::vector<MachineValue> operands, int rangeL = -1, int rangeR = -1);

	static bool isInverted(Equation::Operator_t op);

	/////
	// Constructors/destructors
public:
	EquationComputer() = delete;

};

#endif // EQUATIONCOMPUTER_H
