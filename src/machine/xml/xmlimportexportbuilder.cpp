/*
 * Copyright © 2021-2026 Clément Foucher
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

// StateS
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
std::shared_ptr<MachineXmlWriter> XmlImportExportBuilder::buildMachineWriterForUndoRedo()
{
	std::shared_ptr<MachineXmlWriter> machineWriter;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm != nullptr)
	{
		machineWriter = std::make_shared<FsmXmlWriter>(MachineXmlWriter::WriteMode_t::writeToUndo);
	}

	return machineWriter;
}

/**
 * @brief XmlImportExportBuilder::buildMachineWriterForSaveFile
 * Builds a machine writer that produces XML for save files.
 * It includes view configuration as loading a file recovers view.
 * @param machineManager
 * @param viewConfiguration
 * @return
 */
std::shared_ptr<MachineXmlWriter> XmlImportExportBuilder::buildMachineWriterForSaveFile(std::shared_ptr<ViewConfiguration> viewConfiguration)
{
	std::shared_ptr<MachineXmlWriter> machineWriter;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm != nullptr)
	{
		machineWriter = std::make_shared<FsmXmlWriter>(MachineXmlWriter::WriteMode_t::writeToFile, viewConfiguration);
	}

	return machineWriter;
}

/**
 * @brief XmlImportExportBuilder::buildStringParser
 * Builds a parser for a QString object.
 * @param xmlString
 * @return
 */
std::shared_ptr<MachineXmlParser> XmlImportExportBuilder::buildStringParser(const QString& xmlString)
{
	std::shared_ptr<MachineXmlParser> machineParser;

	StateSXmlAnalyzer analyzer{xmlString};

	if (analyzer.getMachineType() == StateSXmlAnalyzer::MachineType_t::fsm)
	{
		machineParser = std::make_shared<FsmXmlParser>(xmlString);
	}

	return machineParser;
}

/**
 * @brief XmlImportExportBuilder::buildFileParser
 * Builds a parser for a QFile object.
 * @param file
 * @return
 */
std::shared_ptr<MachineXmlParser> XmlImportExportBuilder::buildFileParser(std::shared_ptr<QFile> file, std::shared_ptr<StateSXmlAnalyzer> analyzer)
{
	if (analyzer == nullptr) return nullptr;


	std::shared_ptr<MachineXmlParser> machineParser;

	if (analyzer->getMachineType() == StateSXmlAnalyzer::MachineType_t::fsm)
	{
		machineParser = std::make_shared<FsmXmlParser>(file);
	}

	return machineParser;
}
