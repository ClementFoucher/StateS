/*
 * Copyright © 2014-2023 Clément Foucher
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
class StatesUi;


/**
 * @brief The StatesGraphicsView class ignores drag and drop
 * events so that they can be caught by the StatesUI object.
 */
class StatesGraphicsView : public QGraphicsView
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit StatesGraphicsView(QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent     (QDropEvent* event)      override;

};

#endif // STATESGRAPHICSVIEW_H
