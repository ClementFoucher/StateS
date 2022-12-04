/*
 * Copyright © 2017-2021 Clément Foucher
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

// Current class header
#include "diffundocommand.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machinexmlparser.h"
#include "machinexmlwriter.h"
#include "xmlimportexportbuilder.h"


QString DiffUndoCommand::machineXmlRepresentation;

void DiffUndoCommand::updateXmlRepresentation()
{
	DiffUndoCommand::machineXmlRepresentation = QString();

	shared_ptr<MachineXmlWriter> saveManager = XmlImportExportBuilder::buildMachineWriter(MachineUndoCommand::machineManager);
	if (saveManager != nullptr)
	{
		DiffUndoCommand::machineXmlRepresentation = saveManager->getMachineXml();
	}
}

DiffUndoCommand::DiffUndoCommand(undo_command_id commandId)
{
	undoType = commandId;

	shared_ptr<MachineXmlWriter> saveManager = XmlImportExportBuilder::buildMachineWriter(MachineUndoCommand::machineManager);
	if (saveManager != nullptr)
	{
		// Get code before and after change
		QString previousXmlCode = DiffUndoCommand::machineXmlRepresentation;
		DiffUndoCommand::machineXmlRepresentation = saveManager->getMachineXml();

		// Compute diff
		diff_match_patch diffComputer = diff_match_patch();
		this->undoPatch = diffComputer.patch_make(DiffUndoCommand::machineXmlRepresentation, previousXmlCode);
	}
}

void DiffUndoCommand::undo()
{
	diff_match_patch diffUnroller = diff_match_patch();
	QString currentXmlCode = DiffUndoCommand::machineXmlRepresentation;

	QPair<QString, QVector<bool> > result = diffUnroller.patch_apply(this->undoPatch, currentXmlCode);
	QString previousXmlCode = result.first;

	// Compute redo code on undo to avoid storing unnecessary data
	diff_match_patch diffComputer = diff_match_patch();
	this->redoPatch = diffComputer.patch_make(previousXmlCode, currentXmlCode);

	this->applyPatch(previousXmlCode);

	DiffUndoCommand::machineXmlRepresentation = previousXmlCode;
}

void DiffUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		diff_match_patch diffUnroller = diff_match_patch();

		QPair<QString, QVector<bool> > result = diffUnroller.patch_apply(this->redoPatch, DiffUndoCommand::machineXmlRepresentation);
		QString nextXmlCode = result.first;

		this->redoPatch.clear();

		this->applyPatch(nextXmlCode);
		DiffUndoCommand::machineXmlRepresentation = nextXmlCode;
	}
	else
	{
		// Ignore initial redo automatically applied when pushed in the stack
		this->firstRedoIgnored = true;
	}
}

bool DiffUndoCommand::mergeWith(const QUndoCommand* command)
{
	// FIXME: as there is no transition reference available,
	// moving slider on various transition in a row is also merged.

	const DiffUndoCommand* otherCommand = dynamic_cast<const DiffUndoCommand*>(command);
	bool mergeAccepted = false;
	if (otherCommand != nullptr)
	{
		switch (this->undoType)
		{
		case undo_command_id::fsmUndoMoveConditionSliderId:
		{
			diff_match_patch diffComputer = diff_match_patch();
			QString currentXmlCode = DiffUndoCommand::machineXmlRepresentation;
			QList<Patch> otherUndoPatch = otherCommand->getUndoPatch();

			QString previousXmlCode = diffComputer.patch_apply(otherUndoPatch, currentXmlCode).first;
			QString previousPreviousXmlCode = diffComputer.patch_apply(this->undoPatch, previousXmlCode).first;

			this->undoPatch = diffComputer.patch_make(currentXmlCode, previousPreviousXmlCode);

			mergeAccepted = true;
			break;
		}
		default:
			break;
		}
	}

	return mergeAccepted;
}

QList<Patch> DiffUndoCommand::getUndoPatch() const
{
	return this->undoPatch;
}

void DiffUndoCommand::applyPatch(const QString& newXmlCode)
{
	shared_ptr<MachineXmlParser> parser = XmlImportExportBuilder::buildStringParser(newXmlCode);
	if (parser != nullptr)
	{
		parser->doParse();
		emit applyUndoRedo(parser->getMachine());
	}
}
