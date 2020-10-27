/*
 * Copyright © 2017-2020 Clément Foucher
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

#ifndef MACHINEUNDOCOMMAND_H
#define MACHINEUNDOCOMMAND_H

// Parent classes
#include <QObject>
#include <QUndoCommand>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Machine;


class MachineUndoCommand : public QObject, public QUndoCommand
{
	Q_OBJECT

public:
	// Define command id for mergable undo objects
	enum undo_command_id
	{
		undefinedUndoId = -1,
		machineGenericUndoId = 0,

		// Machine common commands
		machineUndoRenameId = 1,

		// FSM-specific commands
		fsmUndoStateMoveId = 10,
		fsmUndoMoveConditionSliderId = 11,
	};

	static void setMachine(shared_ptr<Machine> machine, bool machineHasChanged);

public:
	explicit MachineUndoCommand();
	explicit MachineUndoCommand(const QString& previousName);

	virtual void undo() override;
	virtual void redo() override;

	virtual int id() const override;

protected:
	static weak_ptr<Machine> machine;

	undo_command_id undoType = undo_command_id::undefinedUndoId;
	bool firstRedoIgnored;

private:
	QString previousName;
	QString nextName;
};

#endif // MACHINEUNDOCOMMAND_H
