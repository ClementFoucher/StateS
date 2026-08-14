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

#ifndef XMLIMPORTEXPORTBUILDER_H
#define XMLIMPORTEXPORTBUILDER_H

// Parent
#include <QObject>

// Stdlib
#include <memory>

// Qt
class QFile;

// StateS
class MachineXmlWriter;
class MachineXmlParser;
class ViewConfiguration;
class StateSXmlAnalyzer;


class XmlImportExportBuilder : public QObject
{
	Q_OBJECT

	/////
	// Static functions
public:
	// Writer
	static std::shared_ptr<MachineXmlWriter> buildMachineWriterForUndoRedo();
	static std::shared_ptr<MachineXmlWriter> buildMachineWriterForSaveFile(std::shared_ptr<ViewConfiguration> viewConfiguration);

	// Parser
	static std::shared_ptr<MachineXmlParser> buildStringParser(const QString& xmlString);
	static std::shared_ptr<MachineXmlParser> buildFileParser(std::shared_ptr<QFile> file, std::shared_ptr<StateSXmlAnalyzer> analyzer);

};

#endif // XMLIMPORTEXPORTBUILDER_H
