/*
 * Copyright © 2014-2015 Clément Foucher
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

// StateS classes
#include "langselectiondialog.h"
#include "states.h"


int main(int argc, char* argv[])
{
    // Create application
    shared_ptr<QApplication> app(new QApplication(argc, argv));

    // Show language seletion dialog and obtain translator from it
    unique_ptr<LangSelectionDialog> langageSelectionWindow(new LangSelectionDialog(app));
    langageSelectionWindow->setWindowFlags(Qt::Dialog | langageSelectionWindow->windowFlags());
    langageSelectionWindow->setWindowTitle("StateS");
    langageSelectionWindow->exec();

    shared_ptr<QTranslator> translator = langageSelectionWindow->getTranslator();

    // This wont be used again in application lifetime
    // Clear associated resources
    langageSelectionWindow.reset();

    // Build StateS main object and begin execution
    unique_ptr<StateS> states(new StateS());
    states->run();

    // Event loop
    int res = app->exec();

    // Clear everyting
    states.reset();
    translator.reset();
    app.reset();

    // The end
    return res;
}
