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

// Current class header
#include "scenewidget.h"

// Debug
#include <QDebug>

// Qt classes
#include <QResizeEvent>
#include <QStyle>
#include <QScrollBar>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

// StateS classes
#include "machinemanager.h"
#include "blankscene.h"
#include "viewconfiguration.h"
#include "graphicmachine.h"
#include "pixmapgenerator.h"
#include "machinebuilder.h"


/////
// Static

const double SceneWidget::scaleFactor = 1.15;
const qreal  SceneWidget::sceneMargin = 100;

/////
// Object

SceneWidget::SceneWidget(QWidget* parent) :
    StatesGraphicsView(parent)
{
	// Connect Machine Manager
	connect(machineManager.get(), &MachineManager::machineReplacedEvent,       this, &SceneWidget::machineReplacedEventHandler);
	connect(machineManager.get(), &MachineManager::simulationModeChangedEvent, this, &SceneWidget::simulationModeChangedEventHandler);

	// Build zoom controls
	this->zoomBackground = new QFrame(this);
	this->labelZoom      = new QLabel(tr("Zoom"), this);
	this->buttonZoomIn   = new QPushButton("+", this);
	this->buttonNoZoom   = new QPushButton("⟳", this);
	this->buttonZoomOut  = new QPushButton("-", this);
	this->buttonZoomFit  = new QPushButton("❊", this);

	this->zoomBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 0.5); border-radius: 5;"));
	this->labelZoom->setStyleSheet(QString("color: white;"));

	this->buttonZoomIn ->setToolTip(tr("Zoom in"));
	this->buttonNoZoom ->setToolTip(tr("Reset zoom"));
	this->buttonZoomOut->setToolTip(tr("Zoom out"));
	this->buttonZoomFit->setToolTip(tr("Zoom to fit machine"));

	this->zoomBackground->resize(50, 100);
	this->buttonZoomIn  ->resize(QSize(20, 20));
	this->buttonNoZoom  ->resize(QSize(20, 20));
	this->buttonZoomOut ->resize(QSize(20, 20));
	this->buttonZoomFit ->resize(QSize(40, 20));

	connect(this->buttonZoomIn,  &QAbstractButton::clicked, this, &SceneWidget::zoomIn);
	connect(this->buttonNoZoom,  &QAbstractButton::clicked, this, &SceneWidget::resetZoom);
	connect(this->buttonZoomOut, &QAbstractButton::clicked, this, &SceneWidget::zoomOut);
	connect(this->buttonZoomFit, &QAbstractButton::clicked, this, &SceneWidget::zoomFit);

	this->zoomBackground->setVisible(false);
	this->labelZoom     ->setVisible(false);
	this->buttonZoomIn  ->setVisible(false);
	this->buttonZoomFit ->setVisible(false);
	this->buttonZoomOut ->setVisible(false);
	this->buttonNoZoom  ->setVisible(false);

	// Build scene
	this->buildScene();
}

GenericScene* SceneWidget::getScene() const
{
	return dynamic_cast<GenericScene*>(this->scene());
}

void SceneWidget::setView(shared_ptr<ViewConfiguration> viewConfiguration)
{
	if (viewConfiguration != nullptr)
	{
		this->setZoomLevel(viewConfiguration->zoomLevel);
		this->centerOn(viewConfiguration->viewCenter);
	}
	else
	{
		this->setZoomLevel(1);
		this->centerOn(QPointF(0, 0));
	}
}

shared_ptr<ViewConfiguration> SceneWidget::getView() const
{
	shared_ptr<ViewConfiguration> viewConfiguration = shared_ptr<ViewConfiguration>(new ViewConfiguration());

	viewConfiguration->viewCenter = this->getVisibleArea().center();
	viewConfiguration->zoomLevel  = this->getZoomLevel();

	auto scene = this->getScene();
	if (scene != nullptr)
	{
		viewConfiguration->sceneTranslation = -(scene->getItemsBoundingRect().topLeft());
	}

	return viewConfiguration;
}

void SceneWidget::clearSelection()
{
	this->getScene()->clearSelection();
}

void SceneWidget::mousePressEvent(QMouseEvent* me)
{
	bool transmitEvent = true;

	if ( (this->sceneMode != SceneMode_t::noScene) && (me->button() == Qt::MiddleButton) )
	{
		this->updateSceneAction(SceneAction_t::movingScene, true);
		transmitEvent = false;
	}

	if (transmitEvent == true)
	{
		QGraphicsView::mousePressEvent(me);
	}
}

void SceneWidget::mouseMoveEvent(QMouseEvent* me)
{
	static QPoint previousMouseEventPos(0, 0);

	bool transmitEvent = true;

	if ( (this->currentAction & SceneAction_t::movingScene) != 0)
	{
		QScrollBar* hBar = horizontalScrollBar();
		QScrollBar* vBar = verticalScrollBar();
		QPoint delta = me->pos() - previousMouseEventPos;
		hBar->setValue(hBar->value() + -delta.x());
		vBar->setValue(vBar->value() - delta.y());

		transmitEvent = false;
	}

	previousMouseEventPos = me->pos();

	if (transmitEvent)
	{
		QGraphicsView::mouseMoveEvent(me);
	}
}

void SceneWidget::mouseReleaseEvent(QMouseEvent* me)
{
	bool transmitEvent = true;

	if ( (this->currentAction & SceneAction_t::movingScene) != 0)
	{
		this->updateSceneAction(SceneAction_t::movingScene, false);
		transmitEvent = false;
	}

	if (transmitEvent)
	{
		QGraphicsView::mouseReleaseEvent(me);
	}
}

void SceneWidget::mouseDoubleClickEvent(QMouseEvent* me)
{
	bool transmitEvent = true;

	if ( (this->currentAction & SceneAction_t::movingScene) != 0)
	{
		transmitEvent = false;
	}

	if (transmitEvent)
	{
		QGraphicsView::mouseMoveEvent(me);
	}
}

void SceneWidget::wheelEvent(QWheelEvent* event)
{
	// Never transmit wheel events to parent: wheel is fully handed at this level

	if (this->sceneMode == SceneMode_t::noScene)
		return;

	if ( (event->modifiers() & Qt::ControlModifier) != 0)
	{
		// Zoom when combined with ctrl
		this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

		if(event->angleDelta().y() > 0)
		{
			scale(scaleFactor, scaleFactor);
		}
		else
		{
			scale(1/scaleFactor, 1/scaleFactor);
		}
	}
	else if ( (event->modifiers() & Qt::ShiftModifier) != 0)
	{
		// Scroll horizontally when combined with shift
		QScrollBar *hBar = horizontalScrollBar();
		hBar->setValue(hBar->value() - event->angleDelta().y());
	}
	else
	{
		// Scroll vertically by default
		QScrollBar *vBar = verticalScrollBar();
		vBar->setValue(vBar->value() - event->angleDelta().y());
	}
}

void SceneWidget::resizeEvent(QResizeEvent* event)
{
	// Relocate overlay buttons (may not be displayed at this time)

	// Base margin
	int topAlign   = 10;
	int rightAlign = 10;
	int backMargin = 5;
	int yDistance  = 10;

	// Include the potential toolbar size to not overlap,
	// but always include it to avoid buttons moving under mouse.
	rightAlign += this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

	int vpos = topAlign + backMargin;
	int xpos = this->width() - this->labelZoom->width() - rightAlign - backMargin;
	this->labelZoom->move(xpos, vpos);

	vpos = vpos + this->labelZoom->height() + yDistance;
	xpos = xpos + this->labelZoom->width()/2 - this->buttonZoomIn->width()/2;
	this->buttonZoomIn->move(xpos, vpos);

	vpos = vpos + this->buttonZoomIn->height() + yDistance;
	xpos = xpos + this->buttonZoomIn->width()/2 - this->buttonNoZoom->width()/2;
	this->buttonNoZoom ->move(xpos, vpos);

	vpos = vpos + this->buttonNoZoom->height() + yDistance;
	xpos = xpos + this->buttonNoZoom->width()/2 - this->buttonZoomOut->width()/2;
	this->buttonZoomOut->move(xpos, vpos);

	vpos = vpos + this->buttonZoomOut->height() + yDistance;
	xpos = xpos + this->buttonZoomOut->width()/2 - this->buttonZoomFit->width()/2;
	this->buttonZoomFit->move(xpos, vpos);

	int backHeight = (this->buttonZoomFit->pos().y() + this->buttonZoomFit->height()) - (this->labelZoom->pos().y()) + 2*backMargin;
	int backWidth  = this->labelZoom->width() + 2*backMargin;
	this->zoomBackground->resize(backWidth, backHeight);
	this->zoomBackground->move(this->width() - backWidth - rightAlign, topAlign);

	// Transmit event
	QGraphicsView::resizeEvent(event);
}

void SceneWidget::machineReplacedEventHandler()
{
	this->clearScene();
	this->buildScene();
}

void SceneWidget::sceneRectChangedEventHandler(QRectF sceneRect)
{
	sceneRect.adjust(-sceneMargin, -sceneMargin, sceneMargin, sceneMargin);
	this->setSceneRect(sceneRect);
}

void SceneWidget::simulationModeChangedEventHandler(SimulationMode_t newMode)
{
	if (newMode == SimulationMode_t::simulateMode)
	{
		this->updateSceneMode(SceneMode_t::simulating);
	}
	else
	{
		this->updateSceneMode(SceneMode_t::editing);
	}
}

void SceneWidget::toolChangedEventHandler(MachineBuilderTool_t tool)
{
	// Tool changes are only handled in edit mode
	if (this->sceneMode != SceneMode_t::editing)
	{
		return;
	}

	// In edit mode, tool change will cause scene mode to change
	switch (tool) {
	case MachineBuilderTool_t::none:
		this->updateMouseCursor(MouseCursor_t::none);
		this->updateSceneAction(SceneAction_t::usingTool, false);
		break;
	case MachineBuilderTool_t::initial_state:
	case MachineBuilderTool_t::state:
		this->updateMouseCursor(MouseCursor_t::state);
		this->updateSceneAction(SceneAction_t::usingTool, true);
		break;
	case MachineBuilderTool_t::transition:
		this->updateMouseCursor(MouseCursor_t::transition);
		this->updateSceneAction(SceneAction_t::usingTool, true);
		break;
	}
}

void SceneWidget::singleUseToolChangedEventHandler(MachineBuilderSingleUseTool_t tool)
{
	// Tool changes are only handled in edit mode
	if (this->sceneMode != SceneMode_t::editing)
	{
		this->updateMouseCursor(MouseCursor_t::none);
		return;
	}

	switch (tool) {
	case MachineBuilderSingleUseTool_t::none:
		this->updateMouseCursor(MouseCursor_t::none);
		this->updateSceneAction(SceneAction_t::usingTool, false);
		break;
	case MachineBuilderSingleUseTool_t::drawTransitionFromScene:
	case MachineBuilderSingleUseTool_t::editTransitionSource:
	case MachineBuilderSingleUseTool_t::editTransitionTarget:
		this->updateSceneAction(SceneAction_t::usingTool, true);
		this->updateMouseCursor(MouseCursor_t::transition);
		break;
	}
}

void SceneWidget::zoomIn()
{
	this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	this->scale(scaleFactor, scaleFactor);
}

void SceneWidget::zoomOut()
{
	this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	this->scale(1/scaleFactor, 1/scaleFactor);
}

void SceneWidget::zoomFit()
{
	auto scene = this->getScene();
	if (scene == nullptr) return;

	QRectF idealView = scene->getItemsBoundingRect();
	if (idealView.isNull() == true) return; // Zoom fit is only relevant if scene contains elements

	QRectF currentView = this->getVisibleArea();

	qreal scaleDiffWidth  = currentView.width()  / idealView.width();
	qreal scaleDiffHeight = currentView.height() / idealView.height();

	qreal scaleDiff = min(scaleDiffWidth, scaleDiffHeight);
	this->scale(scaleDiff, scaleDiff);

	this->centerOn(idealView.center());
}

void SceneWidget::resetZoom()
{
	this->setZoomLevel(1);
}

void SceneWidget::clearScene()
{
	GenericScene* oldScene = this->getScene();
	if (oldScene != nullptr)
	{
		this->setScene(nullptr);
		delete oldScene;
	}

	this->updateSceneMode(SceneMode_t::noScene);
	this->updateSceneAction(SceneAction_t::idle, true);
	this->updateMouseCursor(MouseCursor_t::none);
}

void SceneWidget::buildScene()
{
	// State when this function is called is always noScene/idle/none

	GenericScene* newScene = nullptr;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine != nullptr)
	{
		newScene = graphicMachine->getGraphicScene();
	}

	auto machineBuilder = machineManager->getMachineBuilder();

	if ( (newScene == nullptr) || (machineBuilder == nullptr) )
	{
		this->setScene(new BlankScene());
		this->setView(nullptr);
		return;
	}

	connect(newScene, &GenericScene::itemSelectedEvent,       this, &SceneWidget::itemSelectedEvent);
	connect(newScene, &GenericScene::editSelectedItemEvent,   this, &SceneWidget::editSelectedItemEvent);
	connect(newScene, &GenericScene::renameSelectedItemEvent, this, &SceneWidget::renameSelectedItemEvent);
	connect(newScene, &GenericScene::sceneRectChanged,        this, &SceneWidget::sceneRectChangedEventHandler);

	connect(machineBuilder.get(), &MachineBuilder::changedToolEvent,      this, &SceneWidget::toolChangedEventHandler);
	connect(machineBuilder.get(), &MachineBuilder::singleUseToolSelected, this, &SceneWidget::singleUseToolChangedEventHandler);

	this->setScene(newScene);
	newScene->recomputeSceneRect();
	this->setView(nullptr);
	this->updateSceneMode(SceneMode_t::editing);
}

void SceneWidget::updateSceneMode(SceneMode_t newMode)
{
	if (newMode == this->sceneMode) return; // If no change, ignore call

	// Update mode
	this->sceneMode = newMode;

	// Update zoom buttons visibility
	if (this->sceneMode != SceneMode_t::noScene) // editing or simulating
	{
		this->setZoomPanelVisible(true);
	}
	else // noScene
	{
		this->setZoomPanelVisible(false);
	}

	// Update drag mode
	if (this->sceneMode == SceneMode_t::editing)
	{
		this->setDragMode(QGraphicsView::RubberBandDrag);
	}
	else // noScene or simulating
	{
		this->setDragMode(QGraphicsView::NoDrag);
	}

	// Reset action and cursor when changing mode
	this->updateSceneAction(SceneAction_t::idle, true);
	this->updateMouseCursor(MouseCursor_t::none);
}

void SceneWidget::updateSceneAction(SceneAction_t action, bool enable)
{
	// Update action
	if (action == SceneAction_t::idle)
	{
		this->currentAction = SceneAction_t::idle;
	}
	else if (enable == true)
	{
		this->currentAction |= action;
	}
	else
	{
		this->currentAction &= ~action;
	}

	// Temporary zoom buttons visibility
	if ( (this->currentAction & SceneAction_t::movingScene) != 0)
	{
		this->setZoomPanelVisible(false);
	}
	else // idle or usingTool
	{
		// Depends on mode... duplicated code, can do better.
		if (this->sceneMode != SceneMode_t::noScene) // editing or simulating
		{
			this->setZoomPanelVisible(true);
		}
		else // noScene
		{
			this->setZoomPanelVisible(false);
		}
	}

	// Temporary drag mode
	if ( (this->currentAction & SceneAction_t::movingScene) != 0)
	{
		// Just for mouse icon, not using its properties as it requires left mouse button pressed to work
		this->setDragMode(QGraphicsView::ScrollHandDrag);
	}
	else if ( (this->currentAction & SceneAction_t::usingTool) != 0)
	{
		this->setDragMode(QGraphicsView::NoDrag);
	}
	else // idle
	{
		// Depends on mode... duplicated code, can do better.
		if (this->sceneMode == SceneMode_t::editing)
		{
			this->setDragMode(QGraphicsView::RubberBandDrag);
		}
		else // noScene or simulating
		{
			this->setDragMode(QGraphicsView::NoDrag);
		}
	}
}

void SceneWidget::updateMouseCursor(MouseCursor_t cursor)
{
	switch(cursor)
	{
	case MouseCursor_t::none:
		this->unsetCursor();
		break;
	case MouseCursor_t::state:
		this->setCursor(QCursor(PixmapGenerator::getFsmStateCursor(), 0, 0));
		break;
	case MouseCursor_t::transition:
		this->setCursor(QCursor(PixmapGenerator::getFsmTransitionCursor(), 0, 0));
		break;
	}
}

void SceneWidget::setZoomPanelVisible(bool visible)
{
	this->zoomBackground->setVisible(visible);
	this->labelZoom     ->setVisible(visible);
	this->buttonZoomIn  ->setVisible(visible);
	this->buttonNoZoom  ->setVisible(visible);
	this->buttonZoomOut ->setVisible(visible);
	this->buttonZoomFit ->setVisible(visible);
}

void SceneWidget::setZoomLevel(qreal level)
{
	QTransform scaleMatrix;
	scaleMatrix.scale(level, level);
	this->setTransform(scaleMatrix);
}

qreal SceneWidget::getZoomLevel() const
{
	return this->transform().m11();
}

QRectF SceneWidget::getVisibleArea() const
{
	return this->mapToScene(this->rect()).boundingRect();
}
