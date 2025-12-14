/*
 * Copyright © 2017-2025 Clément Foucher
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
#include "machinexmlparser.h"
#include "machinexmlwriter.h"
#include "xmlimportexportbuilder.h"


/////
// Static members

QString DiffUndoCommand::machineXmlRepresentation;

void DiffUndoCommand::updateXmlRepresentation()
{
	auto machineWriter = XmlImportExportBuilder::buildMachineWriterForUndoRedo();
	if (machineWriter == nullptr) return;


	DiffUndoCommand::machineXmlRepresentation = machineWriter->getMachineXml();
}

/////
// Constructors/destructors

DiffUndoCommand::DiffUndoCommand(const QString& description) :
	StatesUndoCommand(UndoCommandId_t::diffUndoId, description)
{
	auto machineWriter = XmlImportExportBuilder::buildMachineWriterForUndoRedo();
	if (machineWriter == nullptr) return;


	// Get XML before change
	auto previousXmlCode = DiffUndoCommand::machineXmlRepresentation;
	// Update machine XML
	DiffUndoCommand::machineXmlRepresentation = machineWriter->getMachineXml();

	// Compute diff
	this->undoDiff = this->computeDiff(DiffUndoCommand::machineXmlRepresentation, previousXmlCode);
}

/////
// Object functions

bool DiffUndoCommand::isEmpty()
{
	if (this->undoDiff.getEditDistance() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DiffUndoCommand::undo()
{
	auto& currentXmlCode = DiffUndoCommand::machineXmlRepresentation;
	auto previousXmlCode = this->patchString(this->undoDiff, currentXmlCode);

	// Compute redo code on undo to avoid storing unnecessary data
	this->redoDiff = this->computeDiff(previousXmlCode, currentXmlCode);

	this->replaceMachine(previousXmlCode);
}

void DiffUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		auto nextXmlCode = this->patchString(this->redoDiff, DiffUndoCommand::machineXmlRepresentation);

		// Clear redo patch
		this->redoDiff = DtlDiff();

		this->replaceMachine(nextXmlCode);
	}
	else
	{
		// Ignore initial redo automatically applied when pushed in the stack
		this->firstRedoIgnored = true;
	}
}

bool DiffUndoCommand::mergeWith(const QUndoCommand* command)
{
	if (this->text().isNull() == true) return false;

	auto otherCommand = dynamic_cast<const DiffUndoCommand*>(command);
	if (otherCommand == nullptr) return false;

	if (otherCommand->text() != this->text()) return false;


	auto& currentXmlCode = DiffUndoCommand::machineXmlRepresentation;

	auto previousXmlCode         = this->patchString(otherCommand->undoDiff, currentXmlCode);
	auto previousPreviousXmlCode = this->patchString(this->undoDiff,         previousXmlCode);

	this->undoDiff = this->computeDiff(currentXmlCode, previousPreviousXmlCode);

	if (this->isEmpty())
	{
		this->setObsolete(true);
	}

	return true;
}

void DiffUndoCommand::replaceMachine(const QString& newXmlCode)
{
	auto machineParser = XmlImportExportBuilder::buildStringParser(newXmlCode);
	if (machineParser == nullptr) return;


	machineParser->doParse();
	emit this->applyUndoRedo(machineParser->getMachine(), machineParser->getGraphicMachineConfiguration());
}

DiffUndoCommand::DtlDiff DiffUndoCommand::computeDiff(const QString& oldString, const QString& newString) const
{
	auto patch = DtlDiff(oldString.toStdString(), newString.toStdString());
	patch.compose();

	return patch;
}

QString DiffUndoCommand::patchString(const DtlDiff& patch, const QString& string) const
{
	auto patchedString = QString::fromStdString(patch.patch(string.toStdString()));

	return patchedString;
}
