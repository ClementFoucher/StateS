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
 */
class SceneWidget : public StatesGraphicsView
{
    Q_OBJECT

private:
    enum class sceneMode_e { noScene, idle, withTool, quittingTool, movingScene };

public:
    explicit SceneWidget(QWidget* parent = nullptr);

    void setMachine(shared_ptr<Machine> newMachine); // TODO: throw exception
    GenericScene* getScene() const;

    qreal getZoomLevel();
    void setZoomLevel(qreal level);
    QRectF getVisibleArea() const;

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
    void updateSceneMode(sceneMode_e newMode);
    void updateDragMode();

private:
    // References
    weak_ptr<MachineBuilder> machineBuilder;

    // Local widgets
    QLabel*      labelZoom     = nullptr;
    QPushButton* buttonZoomIn  = nullptr;
    QPushButton* buttonNoZoom  = nullptr;
    QPushButton* buttonZoomOut = nullptr;
    QPushButton* buttonZoomFit = nullptr;

    // Local variables
    sceneMode_e sceneMode      = sceneMode_e::noScene;
    sceneMode_e savedSceneMode = sceneMode_e::noScene; // Used to restore correct mode after moving scene

};

#endif // SCENEWIDGET_H
