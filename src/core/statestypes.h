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

#ifndef STATESTYPES_H
#define STATESTYPES_H

// Qt classes
#include <QtTypes>


/////
// Typedefs

typedef ulong componentId_t;


/////
// Constants

const componentId_t nullId = 0;


/////
// Enums

enum class SimulationMode_t              { editMode, simulateMode };
enum class ImageFormat_t                 { pdf, svg, png, jpg };
enum class LeftRight_t                   { left, right };
enum class VerifierSeverityLevel_t       { blocking, structure, tool, hint };
enum class VariableNature_t              { input, output, internal, constant };
enum class MachineXmlWriterMode_t        { writeToFile, writeToUndo };
enum class MachineType_t                 { none, fsm };
enum class MouseCursor_t                 { none, state, transition };
enum class MachineBuilderTool_t          { none, initialState, state, transition };
enum class MachineBuilderSingleUseTool_t { none, drawTransitionFromScene, editTransitionSource, editTransitionTarget };
enum class SimulationBehavior_t          { prepare, immediately, after };

enum class OperandSource_t
{
	variable, // Reference (using componentId) to a variable defined in the machine (including constants)
	equation, // Operand is itself an equation
	constant  // Constant defined on-the-fly in equation editor. Not to confuse with constants defined in the machine, which are variables (try to follow here!).
};

// This enum must always be treated using a switch in order to obtain
// a warning when adding a new member in all places it is used.
enum class OperatorType_t
{
	notOp,  // Not equations always have exactly one operand
	andOp,
	orOp,
	xorOp,
	nandOp,
	norOp,
	xnorOp,
	equalOp, // Equal equations always have exactly two operand and are size one
	diffOp,  // Diff  equations always have exactly two operand and are size one
	extractOp, // Extract equations always have exacly one operand
	concatOp,
	identity // For internal use only, exactly one operand
};

enum class EquationComputationFailureCause_t
{
	nofail,
	nullOperand,
	invalidOperand,
	sizeMismatch,
	missingParameter,
	incorrectParameter
};

enum class ActionOnVariableType_t : uint
{
	none       = 0x0,
	continuous = 0x1,
	pulse      = 0x2,
	set        = 0x4,
	reset      = 0x8,
	assign     = 0x10,
	increment  = 0x20,
	decrement  = 0x40
};

enum class UndoCommandId_t : int32_t
{
	// Default value
	undefinedUndoId = -1,

	// Diff undo is used for all cases that
	// don't have a more specific handler.
	diffUndoId = 0,

	// Machine common commands
	machineRenameUndoId = 1,

	// FSM-specific commands
	fsmStateMoveUndoId = 10,
	fsmTransitionConditionSliderPositionChangeUndoId = 11,
	fsmStateRenamedUndoId = 12
};

#endif // STATESTYPES_H
