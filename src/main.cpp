/*
 * Copyright © 2014-2026 Clément Foucher
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

// C++ classes
#include <memory>

// Qt classes
#include <QApplication>
#include <QFile>
#include <QDebug>

// StateS classes
#include "states.h"
#include "statesexception.h"


void logCrash(QApplication* app, const QString& logText)
{
	const QString logPathBase = app->applicationDirPath() + "/states-crash-";
	uint crashNum = 1;

	QString logPath = logPathBase + QString::number(crashNum) +	".log";
	QFile debugFile{logPath};
	while (debugFile.exists() == true)
	{
		crashNum++;
		logPath = logPathBase + QString::number(crashNum) +	".log";
		debugFile.setFileName(logPath);
	}

	QDebug debug_log{&debugFile};
	debug_log.setAutoInsertSpaces(false);

	bool opened = debugFile.open(QFile::WriteOnly | QFile::Text);
	if (opened == false) return;


	debug_log << "States encountered an error and crashed.\n";
	debug_log << "Please open an issue on https://github.com/ClementFoucher/StateS/issues and report this error along with a short description of what you were doing when it happened.\n";
	debug_log << "The error text is:\n";
	debug_log << logText;
	debug_log << "\n";

	debugFile.close();
}

int main(int argc, char* argv[])
{
	// Create application
	QApplication* app = new QApplication(argc, argv);

	int res;
	try
	{
		// Build StateS main object
		auto states = std::make_unique<StateS>();

		// Start event loop
		res = app->exec();
	}
	catch (const StatesException& e)
	{
		logCrash(app, e.what());

		res = -1;
	}
	catch (const exception& e)
	{
		QString errorText = "The following exception occured in a standard library: ";
		errorText += e.what();
		logCrash(app, errorText);

		res = -1;
	}
	catch (...)
	{
		QString errorText = "An unknown exception occured in an unknown location.";
		logCrash(app, errorText);

		res = -1;
	}

	// Clear application
	delete app;

	// The end
	return res;
}
