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

#ifndef STATESXMLANALYZER_H
#define STATESXMLANALYZER_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QXmlStreamReader;
class QFile;

// StateS classes
#include "statestypes.h"


class StateSXmlAnalyzer : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit StateSXmlAnalyzer(shared_ptr<QFile> file);
	explicit StateSXmlAnalyzer(const QString& xmlSource);

private:
	explicit StateSXmlAnalyzer();

	/////
	// Object functions
public:
	MachineType_t getMachineType();
	QString       getStateSVersion();
	bool          getXmlIsCorrect();

private:
	void parse();

	/////
	// Object variables
private:
	shared_ptr<QXmlStreamReader> xmlReader;

	MachineType_t type;
	QString       version;
	bool          xmlIsCorrect;

};

#endif // STATESXMLANALYZER_H
