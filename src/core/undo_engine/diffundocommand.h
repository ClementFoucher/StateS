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

#ifndef DIFFUNDOCOMMAND_H
#define DIFFUNDOCOMMAND_H

// Parent class
#include "machineundocommand.h"

// C++ classes
#include <memory>
using namespace std;

// Diff Match Patch classes
#include "diff_match_patch.h"

// StateS classes
class Machine;


class DiffUndoCommand : public MachineUndoCommand
{
	Q_OBJECT

public:
	static void updateXmlRepresentation();

public:
	explicit DiffUndoCommand(undo_command_id commandId = machineGenericUndoId);

	virtual void undo() override;
	virtual void redo() override;

	virtual bool mergeWith(const QUndoCommand* command) override;

	QList<Patch> getUndoPatch() const;

signals:
	// As the generic undo command reloads a complete machine,
	// this signal is used to communicate with the main StateS
	// object which monitors it, applying a machine refresh.
	void applyUndoRedo(shared_ptr<Machine> machine);

private:
	void applyPatch(const QString& newXmlCode);

private:
	static QString machineXmlRepresentation;

private:
	QList<Patch> undoPatch;
	QList<Patch> redoPatch;
};

#endif // DIFFUNDOCOMMAND_H
