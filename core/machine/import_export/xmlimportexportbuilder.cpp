/*
 * Copyright © 2021 Clément Foucher
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
#include "machinemanager.h"
#include "statesxmlanalyzer.h"
#include "fsmxmlwriter.h"
#include "fsmxmlparser.h"
#include "fsm.h"


shared_ptr<MachineXmlWriter> XmlImportExportBuilder::buildMachineWriter(shared_ptr<MachineManager> machineManager)
{
	shared_ptr<MachineXmlWriter> machineWriter;

	shared_ptr<Fsm> machineAsFsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (machineAsFsm != nullptr)
	{
		machineWriter = shared_ptr<MachineXmlWriter>(new FsmXmlWriter(machineManager));
	}

	return machineWriter;
}

shared_ptr<MachineXmlParser> XmlImportExportBuilder::buildStringParser(const QString& xmlString)
{
	shared_ptr<MachineXmlParser> machineParser;

	shared_ptr<StateSXmlAnalyzer> analyzer(new StateSXmlAnalyzer(xmlString));

	if (analyzer->getMachineType() == StateSXmlAnalyzer::machineType::Fsm)
	{
		machineParser = shared_ptr<FsmXmlParser>(new FsmXmlParser(xmlString));
	}

	return machineParser;
}

shared_ptr<MachineXmlParser> XmlImportExportBuilder::buildFileParser(shared_ptr<QFile> file)
{
	shared_ptr<MachineXmlParser> machineParser;

	shared_ptr<StateSXmlAnalyzer> analyzer(new StateSXmlAnalyzer(file));

	if (analyzer->getMachineType() == StateSXmlAnalyzer::machineType::Fsm)
	{
		machineParser = shared_ptr<FsmXmlParser>(new FsmXmlParser(file));
	}

	return machineParser;
}
