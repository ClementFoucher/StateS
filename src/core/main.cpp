/*
 * Copyright © 2014-2022 Clément Foucher
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
using namespace std;

// Qt classes
#include <QApplication>

#include <QDebug>

// StateS classes
#include "states.h"
#include "statesexception.h"


// Debug management (inactive for now)
#define DEBUG_TO_FILE 0

#if DEBUG_TO_FILE == 1

#include <QFile>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define DEBUG_LOG_FILE (SHGetSpecialFolderPath(CSIDL_DESKTOP) + "\StateS.log")
#else
#define DEBUG_LOG_FILE "~/StateS.log"
#endif

#endif


int main(int argc, char* argv[])
{
	// Create application
	QApplication* app = new QApplication(argc, argv);

	// Build StateS main object and begin execution
	int res;
	unique_ptr<StateS> states;
	try
	{
		if (argc >= 2)
		{
			states = make_unique<StateS>(app, argv[1]);
		}
		else
		{
			states = make_unique<StateS>(app);
		}

		// Start event loop
		res = app->exec();
	}
	catch (const StatesException& e)
	{
		// TODO: check log + actualize error text
		qDebug() << "Error! " << e.what();
		qDebug() << "Sorry for the inconvenience. Terminating StateS";
#if DEBUG_TO_FILE == 1
		QFile debug_file(DEBUG_LOG_FILE);
		QDebug debug_log(&debug_file);
		debug_log << e.what();
		debug_log << "Please send this file to SateS-dev@outlook.fr with a short description of what you were doing when the error occured.";
		debug_file.close();
#endif
		res = -1;
	}
	catch (const exception& e)
	{
		// TODO: write in log file
		qDebug() << "Error! Unknown exception occured in a standard library.";
		qDebug() << "I wish I knew where.";
		qDebug() << "Exception says: \"" << e.what() << "\".";
		qDebug() << "Sorry for the inconvenience. Terminating StateS";
#if DEBUG_TO_FILE == 1
		QFile debug_file(DEBUG_LOG_FILE);
		QDebug debug_log(&debug_file);
		debug_log << "Unhandled exception occured in a standard library.";
		debug_log << "Exception says: \"" << e.what() << "\".";
		debug_log << "Please send this file to SateS-dev@outlook.fr with a short description of what you were doing when the error occured.";
		debug_file.close();
#endif
		res = -1;
	}
	catch (...)
	{
		// TODO: write in log file
		qDebug() << "Error! Unknown exception occured somewhere.";
		qDebug() << "I wish I knew what and where.";
		qDebug() << "Sorry for the inconvenience. Terminating StateS";
#if DEBUG_TO_FILE == 1
		QFile debug_file(DEBUG_LOG_FILE);
		QDebug debug_log(&debug_file);
		debug_log << "Unknown exception occured.";
		debug_log << "Please send this file to SateS-dev@outlook.fr with a short description of what you were doing when the error occured.";
		debug_file.close();
#endif
		res = -1;
	}

	// Clear everyting
	states.reset();
	delete app;

	// The end
	return res;
}
