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

#ifndef MACHINECOMPONENTVISUALIZER_H
#define MACHINECOMPONENTVISUALIZER_H

// Parent
#include "statesgraphicsview.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QGraphicsScene;
class QGraphicsView;
class QMouseEvent;


class MachineComponentVisualizer : public StatesGraphicsView
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineComponentVisualizer(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	shared_ptr<QGraphicsScene> getComponentVisualizationScene() const;

protected:
	virtual void mousePressEvent      (QMouseEvent* me)    override;
	virtual void mouseMoveEvent       (QMouseEvent* me)    override;
	virtual void mouseReleaseEvent    (QMouseEvent* me)    override;
	virtual void mouseDoubleClickEvent(QMouseEvent* me)    override;
	virtual void wheelEvent           (QWheelEvent* event) override;

private slots:
	void updateMachineVisualization();

	/////
	// Object variables
private:
	shared_ptr<QGraphicsScene> scene;

	// Qwidget with parent
	QGraphicsView* view = nullptr;

	bool isMoving = false;

};

#endif // MACHINECOMPONENTVISUALIZER_H
