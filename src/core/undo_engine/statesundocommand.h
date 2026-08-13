/*
 * Copyright © 2025-2026 Clément Foucher
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

#ifndef STATESUNDOCOMMAND_H
#define STATESUNDOCOMMAND_H

// Parent
#include <QObject>
#include <QUndoCommand>


class StatesUndoCommand : public QObject, public QUndoCommand
{
	Q_OBJECT

	/////
	// Type declarations
protected:
	enum class CommandId_t : int32_t
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

	/////
	// Constructors/destructors
public:
	explicit StatesUndoCommand(CommandId_t undoType) : undoType{undoType} {}
	explicit StatesUndoCommand(CommandId_t undoType, const QString& description) : QUndoCommand(description), undoType{undoType} {}

	virtual ~StatesUndoCommand() = default;

	/////
	// Object functions
public:
	virtual int id() const override;

	/////
	// Object variables
protected:
	bool firstRedoIgnored = false;

private:
	CommandId_t undoType = CommandId_t::undefinedUndoId;

};

#endif // STATESUNDOCOMMAND_H
