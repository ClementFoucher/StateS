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

// StateS classes
#include "machinemanager.h"
#include "fsmscene.h"
#include "fsm.h"
#include "fsmgraphicstate.h"
#include "fsmgraphictransition.h"
#include "blankscene.h"
#include "viewconfiguration.h"


double SceneWidget::scaleFactor = 1.15;

SceneWidget::SceneWidget(shared_ptr<MachineManager> machineManager, QWidget* parent) :
    StatesGraphicsView(parent)
{
	// Set and connect Machine Manager
	this->machineManager = machineManager;
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent,             this, &SceneWidget::machineUpdatedEventHandler);
	connect(machineManager.get(), &MachineManager::machineViewUpdateRequestedEvent, this, &SceneWidget::updateMachineView);

	// Build buttons
	this->labelZoom     = new QLabel(tr("Zoom"), this);
	this->buttonZoomIn  = new QPushButton("+", this);
	this->buttonNoZoom  = new QPushButton("⟳", this);
	this->buttonZoomOut = new QPushButton("-", this);
	this->buttonZoomFit = new QPushButton("❊", this);

	this->buttonZoomIn ->setToolTip(tr("Zoom in"));
	this->buttonNoZoom ->setToolTip(tr("Reset zoom"));
	this->buttonZoomOut->setToolTip(tr("Zoom out"));
	this->buttonZoomFit->setToolTip(tr("Zoom to fit machine"));

	this->buttonZoomIn ->resize(QSize(20, 20));
	this->buttonNoZoom ->resize(QSize(20, 20));
	this->buttonZoomOut->resize(QSize(20, 20));
	this->buttonZoomFit->resize(QSize(40, 20));

	connect(this->buttonZoomIn,  &QAbstractButton::clicked, this, &SceneWidget::zoomIn);
	connect(this->buttonNoZoom,  &QAbstractButton::clicked, this, &SceneWidget::resetZoom);
	connect(this->buttonZoomOut, &QAbstractButton::clicked, this, &SceneWidget::zoomOut);
	connect(this->buttonZoomFit, &QAbstractButton::clicked, this, &SceneWidget::zoomFit);

	this->labelZoom    ->setVisible(false);
	this->buttonZoomIn ->setVisible(false);
	this->buttonZoomFit->setVisible(false);
	this->buttonZoomOut->setVisible(false);
	this->buttonNoZoom ->setVisible(false);

	// Build scene
	shared_ptr<Machine> machine = this->machineManager->getMachine();
	if (machine != nullptr)
	{
		this->buildScene(true);
	}
	else
	{
		this->setScene(new BlankScene());
	}
}

void SceneWidget::clearScene()
{
	GenericScene* oldScene = this->getScene();
	if (oldScene != nullptr)
	{
		this->setScene(nullptr);
		delete oldScene;
	}

	this->updateSceneMode(sceneMode_t::noScene);
}

void SceneWidget::buildScene(bool loadView)
{
	GenericScene* newScene = nullptr;

	shared_ptr<Machine> newMachine = this->machineManager->getMachine();
	if (newMachine != nullptr)
	{
		shared_ptr<Fsm> newFsm = dynamic_pointer_cast<Fsm>(newMachine);

		if (newFsm != nullptr)
		{
			newScene = new FsmScene(this->machineManager);
		}
		else
		{
			qDebug() << "(SceneWidget:) Error! Trying to display unknown type of Machine.";
		}
	}

	if (newScene != nullptr)
	{
		newScene->setDisplaySize(this->size());

		connect(newScene, &GenericScene::itemSelectedEvent,       this, &SceneWidget::itemSelectedEvent);
		connect(newScene, &GenericScene::editSelectedItemEvent,   this, &SceneWidget::editSelectedItemEvent);
		connect(newScene, &GenericScene::renameSelectedItemEvent, this, &SceneWidget::renameSelectedItemEvent);
		connect(newScene, &GenericScene::updateCursorEvent,       this, &SceneWidget::updateMouseCursor);

		this->setScene(newScene);
		this->updateSceneMode(sceneMode_t::idle);
	}
	else
	{
		this->setScene(new BlankScene());
		this->updateSceneMode(sceneMode_t::noScene);
	}

	if (loadView == true)
	{
		shared_ptr<ViewConfiguration> viewConfiguration = this->machineManager->getViewConfiguration();
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
}

void SceneWidget::updateMachineView()
{
	shared_ptr<ViewConfiguration> viewConfiguration = shared_ptr<ViewConfiguration>(new ViewConfiguration());

	viewConfiguration->viewCenter = this->getVisibleArea().center();
	viewConfiguration->zoomLevel  = this->getZoomLevel();
	viewConfiguration->sceneTranslation = -(this->getVisibleArea().topLeft());

	this->machineManager->setViewConfiguration(viewConfiguration);
}

GenericScene* SceneWidget::getScene() const
{
	return dynamic_cast<GenericScene*>(this->scene());
}

void SceneWidget::clearSelection()
{
	this->getScene()->clearSelection();
}

qreal SceneWidget::getZoomLevel() const
{
	return this->transform().m11();
}

void SceneWidget::setZoomLevel(qreal level)
{
	QTransform scaleMatrix;
	scaleMatrix.scale(level, level);
	this->setTransform(scaleMatrix);
}

QRectF SceneWidget::getVisibleArea() const
{
	return this->mapToScene(this->rect()).boundingRect();
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
	int rightAlign = 10;

	// Include the potential toolbar size to not overlap,
	// but always include it to avoid buttons moving under mouse.
	rightAlign += this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

	int vpos = 10;
	int xpos = this->width() - this->labelZoom->width() - rightAlign;
	this->labelZoom->move(xpos, vpos);

	vpos = vpos + this->labelZoom->height() + 10;
	xpos = xpos + this->labelZoom->width()/2 - this->buttonZoomIn->width()/2;
	this->buttonZoomIn->move(xpos, vpos);

	vpos = vpos + this->buttonZoomIn->height() + 10;
	xpos = xpos + this->buttonZoomIn->width()/2 - this->buttonNoZoom->width()/2;
	this->buttonNoZoom ->move(xpos, vpos);

	vpos = vpos + this->buttonNoZoom->height() + 10;
	xpos = xpos + this->buttonNoZoom->width()/2 - this->buttonZoomOut->width()/2;
	this->buttonZoomOut->move(xpos, vpos);

	vpos = vpos + this->buttonZoomOut->height() + 10;
	xpos = xpos + this->buttonZoomOut->width()/2 - this->buttonZoomFit->width()/2;
	this->buttonZoomFit->move(xpos, vpos);

	// Transmit event
	QGraphicsView::resizeEvent(event);
}

void SceneWidget::mousePressEvent(QMouseEvent* me)
{
	bool transmitEvent = true;

	if ( (this->sceneMode != sceneMode_t::noScene) && (me->button() == Qt::MiddleButton) )
	{
		this->previousSceneMode = this->sceneMode;
		this->updateSceneMode(sceneMode_t::movingScene);
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

	if (this->sceneMode == sceneMode_t::movingScene)
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

	if (this->sceneMode == sceneMode_t::movingScene)
	{
		this->updateSceneMode(this->previousSceneMode);
		this->previousSceneMode = sceneMode_t::idle;

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

	if (this->sceneMode == sceneMode_t::movingScene)
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

	if (this->sceneMode != sceneMode_t::noScene)
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

void SceneWidget::machineUpdatedEventHandler(bool isNewMachine)
{
	QPointF center = this->getVisibleArea().center();

	this->clearScene();

	if (isNewMachine == true)
	{
		this->buildScene(true);
	}
	else
	{
		this->buildScene(false);
		this->centerOn(center);
	}
}

void SceneWidget::updateMouseCursor(mouseCursor_t cursor)
{
	QPixmap pixmap;
	switch(cursor)
	{
	case mouseCursor_t::none:
		this->unsetCursor();
		this->updateSceneMode(sceneMode_t::idle);
		break;
	case mouseCursor_t::state:
		pixmap = FsmGraphicState::getPixmap(32, false, true);
		this->setCursor(QCursor(pixmap, 0, 0));
		this->updateSceneMode(sceneMode_t::withTool);
		break;
	case mouseCursor_t::transition:
		pixmap = FsmGraphicTransition::getPixmap(32);
		this->setCursor(QCursor(pixmap, 0, 0));
		this->updateSceneMode(sceneMode_t::withTool);
		break;
	}
}

void SceneWidget::updateSceneMode(sceneMode_t newMode)
{
	if (newMode != this->sceneMode)
	{
		this->sceneMode = newMode;

		// Update zoom buttons visibility
		if ( (this->sceneMode == sceneMode_t::idle) || (this->sceneMode == sceneMode_t::withTool) )
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

	if (this->sceneMode == sceneMode_t::idle)
	{
		dragMode = QGraphicsView::RubberBandDrag;
	}
	else if (this->sceneMode == sceneMode_t::movingScene)
	{
		dragMode = QGraphicsView::ScrollHandDrag; // Just for mouse icon, not using its properties as it requires left mouse button pressed to work
	}

	this->setDragMode(dragMode);
}

void SceneWidget::setZoomPanelVisible(bool visible)
{
	this->labelZoom    ->setVisible(visible);
	this->buttonZoomIn ->setVisible(visible);
	this->buttonNoZoom ->setVisible(visible);
	this->buttonZoomOut->setVisible(visible);
	this->buttonZoomFit->setVisible(visible);
}
