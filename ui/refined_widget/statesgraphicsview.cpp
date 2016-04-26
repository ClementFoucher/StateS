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

// Current class header
#include "statesgraphicsview.h"


StatesGraphicsView::StatesGraphicsView(QWidget* parent) :
    QGraphicsView(parent)
{

}

void StatesGraphicsView::dragEnterEvent(QDragEnterEvent* event)
{
    // Search for main window and redirect call
    StatesUi* mainWindow = this->getMainWindow();

    if (mainWindow != nullptr)
        mainWindow->dragEnterEvent(event);
}

void StatesGraphicsView::dropEvent(QDropEvent* event)
{
    // Search for main window and redirect call
    StatesUi* mainWindow = this->getMainWindow();

    if (mainWindow != nullptr)
        mainWindow->dropEvent(event);
}

StatesUi* StatesGraphicsView::getMainWindow()
{
    QObject* parent = this->parent();
    StatesUi* mainWindow = static_cast<StatesUi*>(parent);

    while (mainWindow == nullptr)
    {
        // Check this first in case the widget doesn't have any parent
        if (parent == nullptr)
            break;

        parent = parent->parent();
        mainWindow = static_cast<StatesUi*>(parent);
    }

    return mainWindow;
}
