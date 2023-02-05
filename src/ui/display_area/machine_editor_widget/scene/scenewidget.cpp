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


/////
// Static

double SceneWidget::scaleFactor = 1.15;


/////
// Object

SceneWidget::SceneWidget(QWidget* parent) :
    StatesGraphicsView(parent)
{
	// Connect Machine Manager
	connect(machineManager.get(), &MachineManager::machineReplacedEvent, this, &SceneWidget::machineReplacedEventHandler);

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
	auto machine = machineManager->getMachine();
	if (machine != nullptr)
	{
		this->buildScene();
		this->setView(nullptr);
	}
	else
	{
		this->setScene(new BlankScene());
	}
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
	viewConfiguration->sceneTranslation = -(this->getVisibleArea().topLeft());

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
		this->previousSceneMode = this->sceneMode;
		this->updateSceneMode(SceneMode_t::movingScene);
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

	if (this->sceneMode == SceneMode_t::movingScene)
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

	if (this->sceneMode == SceneMode_t::movingScene)
	{
		this->updateSceneMode(this->previousSceneMode);
		this->previousSceneMode = SceneMode_t::idle;

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

	if (this->sceneMode == SceneMode_t::movingScene)
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
	// In this function, never transmit event: we always handle it, even by doing nothing

	if (this->sceneMode != SceneMode_t::noScene)
	{
		if ( (event->modifiers() & Qt::ControlModifier) != 0)
		{
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
			QScrollBar *hBar = horizontalScrollBar();
			hBar->setValue(hBar->value() - event->angleDelta().y());
		}
		else
		{
			QScrollBar *vBar = verticalScrollBar();
			vBar->setValue(vBar->value() - event->angleDelta().y());
		}
	}
}

void SceneWidget::resizeEvent(QResizeEvent* event)
{
	// Inform scene that view has been modified
	GenericScene* currentScene = this->getScene();
	if (currentScene != nullptr)
	{
		currentScene->setDisplaySize(event->size());
	}

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

	// Reset cursor in case there was a tool selected previously
	this->updateMouseCursor(MouseCursor_t::none);
}

void SceneWidget::updateMouseCursor(MouseCursor_t cursor)
{
	switch(cursor)
	{
	case MouseCursor_t::none:
		this->unsetCursor();
		this->updateSceneMode(SceneMode_t::idle);
		break;
	case MouseCursor_t::state:
		this->setCursor(QCursor(PixmapGenerator::getFsmStateCursor(), 0, 0));
		this->updateSceneMode(SceneMode_t::withTool);
		break;
	case MouseCursor_t::transition:
		this->setCursor(QCursor(PixmapGenerator::getFsmTransitionCursor(), 0, 0));
		this->updateSceneMode(SceneMode_t::withTool);
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
	QRectF idealView = this->getScene()->itemsBoundingRect();

	if (! idealView.isNull()) // Zoom fit is only relevant if scene contains elements
	{
		QRectF currentView = this->getVisibleArea();

		qreal scaleDiffWidth  = currentView.width()  / idealView.width();
		qreal scaleDiffHeight = currentView.height() / idealView.height();

		qreal scaleDiff = min(scaleDiffWidth, scaleDiffHeight);
		this->scale(scaleDiff, scaleDiff);

		this->centerOn(idealView.center());

		// Add comfortable margin
		this->zoomOut();
		this->zoomOut();
	}
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
}

void SceneWidget::buildScene()
{
	GenericScene* newScene = nullptr;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine != nullptr)
	{
		newScene = graphicMachine->getGraphicScene();
	}

	if (newScene != nullptr)
	{
		newScene->setDisplaySize(this->size());

		connect(newScene, &GenericScene::itemSelectedEvent,       this, &SceneWidget::itemSelectedEvent);
		connect(newScene, &GenericScene::editSelectedItemEvent,   this, &SceneWidget::editSelectedItemEvent);
		connect(newScene, &GenericScene::renameSelectedItemEvent, this, &SceneWidget::renameSelectedItemEvent);
		connect(newScene, &GenericScene::updateCursorEvent,       this, &SceneWidget::updateMouseCursor);

		this->setScene(newScene);
		this->updateSceneMode(SceneMode_t::idle);
	}
	else
	{
		this->setScene(new BlankScene());
		this->updateSceneMode(SceneMode_t::noScene);
	}
}

void SceneWidget::updateSceneMode(SceneMode_t newMode)
{
	if (newMode != this->sceneMode)
	{
		this->sceneMode = newMode;

		// Update zoom buttons visibility
		if ( (this->sceneMode == SceneMode_t::idle) || (this->sceneMode == SceneMode_t::withTool) )
		{
			this->setZoomPanelVisible(true);
		}
		else
		{
			this->setZoomPanelVisible(false);
		}

		this->updateDragMode();
	}
}

void SceneWidget::updateDragMode()
{
	QGraphicsView::DragMode dragMode = QGraphicsView::NoDrag;

	if (this->sceneMode == SceneMode_t::idle)
	{
		dragMode = QGraphicsView::RubberBandDrag;
	}
	else if (this->sceneMode == SceneMode_t::movingScene)
	{
		dragMode = QGraphicsView::ScrollHandDrag; // Just for mouse icon, not using its properties as it requires left mouse button pressed to work
	}

	this->setDragMode(dragMode);
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
