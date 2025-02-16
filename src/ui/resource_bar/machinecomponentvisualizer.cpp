/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "machinecomponentvisualizer.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QGraphicsItem>
#include <QScrollBar>

// StateS classes
#include "machinemanager.h"
#include "graphicmachine.h"


MachineComponentVisualizer::MachineComponentVisualizer(QWidget* parent) :
    StatesGraphicsView(parent)
{
	this->scene = shared_ptr<QGraphicsScene>(new QGraphicsScene());

	this->setDragMode(QGraphicsView::ScrollHandDrag);
	this->setScene(this->scene.get());

	this->updateMachineVisualization();

	connect(machineManager.get(), &MachineManager::machineNameChangedEvent,               this, &MachineComponentVisualizer::updateMachineVisualization);
	connect(machineManager.get(), &MachineManager::machineInputVariableListChangedEvent,  this, &MachineComponentVisualizer::updateMachineVisualization);
	connect(machineManager.get(), &MachineManager::machineOutputVariableListChangedEvent, this, &MachineComponentVisualizer::updateMachineVisualization);
}

shared_ptr<QGraphicsScene> MachineComponentVisualizer::getComponentVisualizationScene() const
{
	return this->scene;
}

void MachineComponentVisualizer::mousePressEvent(QMouseEvent* me)
{
	bool transmitEvent = true;

	if ( (me->button() == Qt::LeftButton) || (me->button() == Qt::MiddleButton) )
	{
		this->isMoving = true;
		transmitEvent = false;
	}

	if (transmitEvent)
		QGraphicsView::mousePressEvent(me);
}

void MachineComponentVisualizer::mouseMoveEvent(QMouseEvent* me)
{
	static QPoint lastMouseEventPos(0, 0);

	bool transmitEvent = true;

	if (this->isMoving == true)
	{
		QScrollBar* hBar = horizontalScrollBar();
		QScrollBar* vBar = verticalScrollBar();
		QPoint delta = me->pos() - lastMouseEventPos;
		hBar->setValue(hBar->value() + -delta.x());
		vBar->setValue(vBar->value() - delta.y());

		transmitEvent = false;
	}

	lastMouseEventPos = me->pos();

	if (transmitEvent)
		QGraphicsView::mouseMoveEvent(me);
}

void MachineComponentVisualizer::mouseReleaseEvent(QMouseEvent* me)
{
	bool transmitEvent = true;

	if (this->isMoving == true)
	{
		this->isMoving = false;
		transmitEvent = false;
	}

	if (transmitEvent)
		QGraphicsView::mouseReleaseEvent(me);
}

void MachineComponentVisualizer::mouseDoubleClickEvent(QMouseEvent* me)
{
	bool transmitEvent = true;

	if (this->isMoving == true)
	{
		transmitEvent = false;
	}

	if (transmitEvent)
		QGraphicsView::mouseMoveEvent(me);
}

void MachineComponentVisualizer::wheelEvent(QWheelEvent* event)
{
	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	double scaleFactor = 1.15;
	if(event->angleDelta().y() > 0)
	{
		this->scale(scaleFactor, scaleFactor);
	}
	else
	{
		this->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
	}
}

void MachineComponentVisualizer::updateMachineVisualization()
{
	this->scene->clear();

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	QGraphicsItem* component = graphicMachine->getComponentVisualization();
	this->scene->addItem(component);
}
