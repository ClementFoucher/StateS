/*
 * Copyright © 2017-2023 Clément Foucher
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

// StateS classes
#include "statestypes.h"


class MachineUndoCommand : public QObject, public QUndoCommand
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineUndoCommand();
	explicit MachineUndoCommand(const QString& previousName);

	/////
	// Object functions
	virtual void undo() override;
	virtual void redo() override;

	virtual int id() const override;

protected:
	UndoCommandId_t undoType = UndoCommandId_t::undefinedUndoId;
	bool firstRedoIgnored = false;

	/////
	// Object variables
private:
	QString previousName;
	QString nextName;

};

#endif // MACHINEUNDOCOMMAND_H
