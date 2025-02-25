/*
 * Copyright © 2023-2025 Clément Foucher
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

#ifndef EXCEPTIONTYPES_H
#define EXCEPTIONTYPES_H


typedef enum
{
	unsupported_char = 0,
	resized_to_0     = 1,
	outside_range    = 2
} LogicValueError_t;

typedef enum
{
	out_of_range = 0
} MachineActuatorComponentError_t;

typedef enum
{
	illegal_type              = 0,
	illegal_value             = 1,
	illegal_range             = 2,
	action_value_is_read_only = 3
} ActionOnVariableError_t;

typedef enum
{
	change_current_requested = 0
} ConstantError_t;

typedef enum
{
	building_zero_sized   = 0,
	variable_resized_to_0 = 1,
	size_mismatch         = 2
} VariableError_t;

typedef enum
{
	unknown_variable = 0,
	impossible_error = 1
} MachineError_t;

typedef enum
{
	unable_to_replace = 0,
	unkown_directory  = 1,
	unable_to_open    = 2
} MachineaveFileManagerError_t;

typedef enum
{
	reference_expired = 0
} TruthTableError_t;


#endif // EXCEPTIONTYPES_H
