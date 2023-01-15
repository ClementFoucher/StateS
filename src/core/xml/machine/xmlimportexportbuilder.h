/*
 * Copyright © 2021-2023 Clément Foucher
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

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QFile;

// StateS classes
class MachineXmlWriter;
class MachineXmlParser;
class ViewConfiguration;


class XmlImportExportBuilder : public QObject
{
	Q_OBJECT

	/////
	// Static functions
public:
	// Writer
	static shared_ptr<MachineXmlWriter> buildMachineWriterForUndoRedo();
	static shared_ptr<MachineXmlWriter> buildMachineWriterForSaveFile(shared_ptr<ViewConfiguration> viewConfiguration);

	// Parser
	static shared_ptr<MachineXmlParser> buildStringParser(const QString& xmlString);
	static shared_ptr<MachineXmlParser> buildFileParser  (shared_ptr<QFile> file);

};

#endif // XMLIMPORTEXPORTBUILDER_H
