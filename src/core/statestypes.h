/*
 * Copyright © 2023-2024 Clément Foucher
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
// Enums

enum class SimulationMode_t              { editMode, simulateMode };
enum class ImageFormat_t                 { pdf, svg, png, jpg };
enum class LeftRight_t                   { left, right };
enum class VerifierSeverityLevel_t       { blocking, structure, tool, hint };
enum class ActionOnSignalType_t          { activeOnState, pulse, set, reset, assign, increment, decrement };
enum class SignalType_t                  { Input, Output, LocalVariable, Constant };
enum class MachineXmlWriterMode_t        { writeToFile, writeToUndo };
enum class MachineType_t                 { None, Fsm };
enum class MouseCursor_t                 { none, state, transition };
enum class MachineBuilderTool_t          { none, initial_state, state, transition };
enum class MachineBuilderSingleUseTool_t { none, drawTransitionFromScene, editTransitionSource, editTransitionTarget };


// This enum must always be treated using a switch in order to obtain
// a warning when adding a new member in all places it is used.
enum class EquationNature_t
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
	constant, // To allow dynamically creating constants (which are not machine signals)
	identity // For internal use only, exactly one operand
};

enum class EquationComputationFailureCause_t
{
	uncomputed,
	nofail,
	nullOperand,
	incompleteOperand,
	sizeMismatch,
	missingParameter,
	incorrectParameter,
	notImplemented
};

enum class actuatorAllowedActionType_t : uint
{
	none          = 0x0,
	activeOnState = 0x1,
	pulse         = 0x2,
	set           = 0x4,
	reset         = 0x8,
	assign        = 0x10,
	increment     = 0x20,
	decrement     = 0x40
};

enum class UndoCommandId_t : int32_t
{
	// Default value
	undefinedUndoId = -1,

	// Diff undo (TODO: rename?)
	machineGenericUndoId = 0,

	// Machine common commands
	machineUndoRenameId = 1,

	// FSM-specific commands
	fsmUndoStateMoveId = 10,
	fsmUndoTransitionConditionSliderPositionChangeId = 11,
	fsmUndoStateRenamedId = 12
};


#endif // STATESTYPES_H
