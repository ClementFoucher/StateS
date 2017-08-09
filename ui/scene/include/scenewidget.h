/*
 * Copyright © 2014-2017 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QPushButton;
class QLabel;

// For enum
#include "machinebuilder.h"

// StateS classes
class Machine;
class MachineComponent;
class GenericScene;


/**
 * @brief The SceneWidget class displays a graphic scene
 * linked to a machine. Replacing the machine makes the
 * SceneWidget replace its graphic scene.
 * This class is also in charge of the zoom level UI,
 * and adapts the cursor depending on actions.
 */
class SceneWidget : public StatesGraphicsView
{
	Q_OBJECT

private:
	enum class sceneMode_e { noScene, idle, movingScene };
	enum class mouseCursor_e { none, state, transition };

	static double scaleFactor;

public:
	explicit SceneWidget(QWidget* parent = nullptr);

	void setMachine(shared_ptr<Machine> newMachine, bool maintainView); // TODO: throw exception

	GenericScene* getScene() const;

	QRectF getVisibleArea() const;

	qreal getZoomLevel() const;
	void  setZoomLevel(qreal level);

signals:
	void itemSelectedEvent(shared_ptr<MachineComponent> component);
	void editSelectedItemEvent();
	void renameSelectedItemEvent();

protected:
	void mousePressEvent      (QMouseEvent*)  override;
	void mouseMoveEvent       (QMouseEvent*)  override;
	void mouseReleaseEvent    (QMouseEvent*)  override;
	void mouseDoubleClickEvent(QMouseEvent*)  override;
	void resizeEvent          (QResizeEvent*) override;
	void wheelEvent           (QWheelEvent*)  override;

private slots:
	void toolChangedEventHandler(MachineBuilder::tool newTool);
	void singleUseToolChangedEventHandler(MachineBuilder::singleUseTool newTool);

	void zoomIn();
	void zoomOut();
	void zoomFit();
	void resetZoom();

private:
	void updateTool(MachineBuilder::tool newTool);
	void updateMouseCursor(mouseCursor_e cursor);
	void updateSceneMode(sceneMode_e newMode);
	void updateDragMode();
	void setZoomPanelVisible(bool visible);

private:
	// Zoom panel
	QLabel*      labelZoom     = nullptr;
	QPushButton* buttonZoomIn  = nullptr;
	QPushButton* buttonNoZoom  = nullptr;
	QPushButton* buttonZoomOut = nullptr;
	QPushButton* buttonZoomFit = nullptr;

	// Current state
	sceneMode_e   sceneMode     = sceneMode_e::noScene;
	mouseCursor_e currentCursor = mouseCursor_e::none;

	// Connections
	QMetaObject::Connection machineBuilderChangedToolEventConnection;
	QMetaObject::Connection machineBuilderSingleUseToolSelectedConnection;
};

#endif // SCENEWIDGET_H
