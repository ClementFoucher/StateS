/*
 * Copyright © 2014-2026 Clément Foucher
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

#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

// Parent
#include "statesgraphicsview.h"

// Stdlib
#include <memory>
using namespace std;

// Qt
class QPushButton;
class QLabel;

// StateS
#include "machinemanager.h"
#include "machinebuilder.h"
class GenericScene;
class ViewConfiguration;


/**
 * @brief The SceneWidget class handles the graphic scene,
 * which shows the machine. Replacing the machine makes the
 * SceneWidget replace its graphic scene.
 * This class is in charge of the zoom level and scrolling
 * of the scene. It displays a the zoom UI, and adapts
 * the mouse cursor depending on actions.
 */
class SceneWidget : public StatesGraphicsView
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class SceneMode_t   { noScene, editing, viewing };
	enum class MouseCursor_t { none, state, transition };

	enum class SceneAction_t : uint32_t { idle = 0, movingScene = 1, usingTool = 2};

	/////
	// Static variables
private:
	static const double scaleFactor;
	static const qreal  sceneMargin;

	/////
	// Constructors/destructors
public:
	explicit SceneWidget(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	GenericScene* getScene() const;

	void setView(shared_ptr<ViewConfiguration> viewConfiguration);
	shared_ptr<ViewConfiguration> getView() const;
	void clearSelection();

protected:
	virtual void mousePressEvent      (QMouseEvent*)  override;
	virtual void mouseMoveEvent       (QMouseEvent*)  override;
	virtual void mouseReleaseEvent    (QMouseEvent*)  override;
	virtual void mouseDoubleClickEvent(QMouseEvent*)  override;
	virtual void wheelEvent           (QWheelEvent*)  override;
	virtual void resizeEvent          (QResizeEvent*) override;

private slots:
	void machineReplacedEventHandler();
	void sceneRectChangedEventHandler(QRectF sceneRect);
	void interfaceModeChangedEventHandler(MachineManager::InterfaceMode_t newMode);
	void requestSaveViewEventHandler();
	void requestRestoreViewEventHandler();

	void toolChangedEventHandler(MachineBuilder::Tool_t tool);
	void singleUseToolChangedEventHandler(MachineBuilder::SingleUseTool_t tool);

	void zoomIn();
	void zoomOut();
	void zoomFit();
	void resetZoom();

private:
	void clearScene();
	void buildScene();
	void updateSceneAction(SceneAction_t action, bool enable);
	void updateMouseCursor(MouseCursor_t cursor);
	void setZoomPanelVisible(bool visible);
	void setZoomLevel(qreal level);
	qreal getZoomLevel() const;
	QRectF getVisibleArea() const;

	/////
	// Signals
signals:
	void itemSelectedEvent(ComponentId componentId);
	void editSelectedItemEvent();
	void renameSelectedItemEvent();

	/////
	// Object variables
private:
	// Zoom panel
	QFrame*      zoomBackground = nullptr;
	QLabel*      labelZoom      = nullptr;
	QPushButton* buttonZoomIn   = nullptr;
	QPushButton* buttonNoZoom   = nullptr;
	QPushButton* buttonZoomOut  = nullptr;
	QPushButton* buttonZoomFit  = nullptr;

	// Current state
	SceneMode_t   sceneMode     = SceneMode_t::noScene;
	uint32_t      currentAction = static_cast<uint32_t>(SceneAction_t::idle);
	MouseCursor_t currentCursor = MouseCursor_t::none;

	// Temporary state
	shared_ptr<ViewConfiguration> viewConfigurationSave;

};

#endif // SCENEWIDGET_H
