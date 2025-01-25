/*
 * Copyright © 2021-2025 Clément Foucher
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

// Current class header
#include "xmlimportexportbuilder.h"

// StateS classes
#include "fsmxmlwriter.h"
#include "fsmxmlparser.h"
#include "machinexmlwriter.h"
#include "statesxmlanalyzer.h"
#include "machinemanager.h"
#include "fsm.h"


/**
 * @brief XmlImportExportBuilder::buildMachineWriterForUndoRedo
 * Builds a machine writer without a view configuration for undo/redo commmand.
 * View Configuration is not used as view doesn't change in that case.
 * @param machineManager
 * @return
 */
shared_ptr<MachineXmlWriter> XmlImportExportBuilder::buildMachineWriterForUndoRedo()
{
	shared_ptr<MachineXmlWriter> machineWriter;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm != nullptr)
	{
		machineWriter = shared_ptr<MachineXmlWriter>(new FsmXmlWriter(MachineXmlWriterMode_t::writeToUndo));
	}

	return machineWriter;
}

/**
 * @brief XmlImportExportBuilder::buildMachineWriterForSaveFile
 * Builds a machine writer that produces XML for save files.
 * It includes view configuration as loading a file recovers view.s
 * @param machineManager
 * @param viewConfiguration
 * @return
 */
shared_ptr<MachineXmlWriter> XmlImportExportBuilder::buildMachineWriterForSaveFile(shared_ptr<ViewConfiguration> viewConfiguration)
{
	shared_ptr<MachineXmlWriter> machineWriter;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm != nullptr)
	{
		machineWriter = shared_ptr<MachineXmlWriter>(new FsmXmlWriter(MachineXmlWriterMode_t::writeToFile, viewConfiguration));
	}

	return machineWriter;
}

/**
 * @brief XmlImportExportBuilder::buildStringParser
 * Builds a parser for a QString object.
 * @param xmlString
 * @return
 */
shared_ptr<MachineXmlParser> XmlImportExportBuilder::buildStringParser(const QString& xmlString)
{
	shared_ptr<MachineXmlParser> machineParser;

	shared_ptr<StateSXmlAnalyzer> analyzer(new StateSXmlAnalyzer(xmlString));

	if (analyzer->getMachineType() == MachineType_t::fsm)
	{
		machineParser = shared_ptr<FsmXmlParser>(new FsmXmlParser(xmlString));
	}

	return machineParser;
}

/**
 * @brief XmlImportExportBuilder::buildFileParser
 * Builds a parser for a QFile object.
 * @param file
 * @return
 */
shared_ptr<MachineXmlParser> XmlImportExportBuilder::buildFileParser(shared_ptr<QFile> file)
{
	shared_ptr<MachineXmlParser> machineParser;

	shared_ptr<StateSXmlAnalyzer> analyzer(new StateSXmlAnalyzer(file));

	if (analyzer->getMachineType() == MachineType_t::fsm)
	{
		machineParser = shared_ptr<FsmXmlParser>(new FsmXmlParser(file));
	}

	return machineParser;
}
