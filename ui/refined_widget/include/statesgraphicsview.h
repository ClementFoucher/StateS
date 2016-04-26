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

#ifndef STATESGRAPHICSVIEW_H
#define STATESGRAPHICSVIEW_H

// Parent
#include <QGraphicsView>

// StateS classes
#include "statesui.h"

/**
 * @brief The StatesGraphicsView class adds drag-n-drop allowance
 * to allow dropping files on the whole UI.
 */
class StatesGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit StatesGraphicsView(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent     (QDropEvent* event)      override;

private:
    StatesUi* getMainWindow();

};

#endif // STATESGRAPHICSVIEW_H
