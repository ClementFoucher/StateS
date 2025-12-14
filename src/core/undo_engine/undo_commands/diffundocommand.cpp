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

void DiffUndoCommand::buildXmlRepresentation()
{
	if (DiffUndoCommand::machineXmlRepresentation.isEmpty() == false) return;

	auto machineWriter = XmlImportExportBuilder::buildMachineWriterForUndoRedo();
	if (machineWriter == nullptr) return;


	DiffUndoCommand::machineXmlRepresentation = machineWriter->getMachineXml();
}

void DiffUndoCommand::clearXmlRepresentation()
{
	DiffUndoCommand::machineXmlRepresentation.clear();
}

QStringList DiffUndoCommand::getMachineXmlAsStringList()
{
	if (DiffUndoCommand::machineXmlRepresentation.isEmpty() == true)
	{
		DiffUndoCommand::buildXmlRepresentation();
	}

	return DiffUndoCommand::machineXmlRepresentation.split('\n');
}

/////
// Constructors/destructors

DiffUndoCommand::DiffUndoCommand(const QString& description) :
	StatesUndoCommand(UndoCommandId_t::diffUndoId, description)
{
	// Get XML before changes
	auto previousXmlCode = this->getMachineXmlAsStringList();

	// Clear XML so that it may be rebuilt
	DiffUndoCommand::clearXmlRepresentation();

	// Get updated XML after changes
	auto currentXmlCode = this->getMachineXmlAsStringList();

	// Compute diff
	bool isDiffEmpty;
	this->undoDiff = this->computeDiff(currentXmlCode, previousXmlCode, isDiffEmpty);

	if (isDiffEmpty == true)
	{
		this->setObsolete(true);
	}
}

/////
// Object functions

void DiffUndoCommand::undo()
{
	auto currentXmlCode = this->getMachineXmlAsStringList();
	auto previousXmlCode = this->patchString(this->undoDiff, currentXmlCode);

	// Compute redo code on undo to avoid storing unnecessary data
	bool ignored;
	this->redoDiff = this->computeDiff(previousXmlCode, currentXmlCode, ignored);

	this->replaceMachine(previousXmlCode);
}

void DiffUndoCommand::redo()
{
	if (this->firstRedoIgnored == true)
	{
		auto currentXmlCode = this->getMachineXmlAsStringList();
		auto nextXmlCode = this->patchString(this->redoDiff, currentXmlCode);

		// Clear redo patch
		this->redoDiff = DtlUniVector();

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


	auto currentXmlCode = this->getMachineXmlAsStringList();

	auto previousXmlCode         = this->patchString(otherCommand->undoDiff, currentXmlCode);
	auto previousPreviousXmlCode = this->patchString(this->undoDiff,         previousXmlCode);

	bool isDiffEmpty;
	this->undoDiff = this->computeDiff(currentXmlCode, previousPreviousXmlCode, isDiffEmpty);

	if (isDiffEmpty == true)
	{
		this->setObsolete(true);
	}

	return true;
}

void DiffUndoCommand::replaceMachine(const QStringList& newXmlCode)
{
	auto machineParser = XmlImportExportBuilder::buildStringParser(newXmlCode.join(""));
	if (machineParser == nullptr) return;


	machineParser->doParse();
	emit this->applyUndoRedo(machineParser->getMachine(), machineParser->getGraphicMachineConfiguration());
}

DiffUndoCommand::DtlUniVector DiffUndoCommand::computeDiff(const QStringList& oldString, const QStringList& newString, bool& isDiffEmpty) const
{
	auto patch = DtlDiff(oldString, newString);
	patch.compose();
	patch.composeUnifiedHunks();

	isDiffEmpty = (patch.getEditDistance() == 0);
	return patch.getUniHunks();
}

QStringList DiffUndoCommand::patchString(const DtlUniVector& hunkVector, const QStringList& string) const
{
	auto patch = DtlDiff(hunkVector);

	auto patchedString = patch.uniPatch(string);

	return patchedString;
}
