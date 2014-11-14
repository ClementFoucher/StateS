/*
 * Copyright © 2014 Clément Foucher
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

#include <QGraphicsView>

#include <QPushButton>

class Machine;
class ResourcesBar;

class SceneWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SceneWidget(Machine *machine, ResourcesBar *resources, QWidget* parent);
    explicit SceneWidget(QWidget* parent);

    void setMachine(Machine* Machine, ResourcesBar *resources);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void enterEvent(QEvent*) override;

private slots:
    void zoomIn();
    void zoomOut();

private:
    ResourcesBar* resourcesBar = nullptr;

    QPushButton* buttonZoomIn = nullptr;
    QPushButton* buttonZoomOut = nullptr;

    bool movingScene = false;
};

#endif // SCENEWIDGET_H
