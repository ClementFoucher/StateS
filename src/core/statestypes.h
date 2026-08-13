/*
 * Copyright © 2023-2026 Clément Foucher
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

// Qt
#include <QtTypes>


/////
// Typedefs

typedef ulong componentId_t;


/////
// Constants

const componentId_t nullId = 0;


/////
// Enums

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

#endif // STATESTYPES_H
