/*
 * Copyright © 2014-2015 Clément Foucher
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
#include <QGraphicsView>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QPushButton>

// StateS classes
class Machine;
class ResourceBar;

class SceneWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SceneWidget(shared_ptr<Machine> machine, ResourceBar *resources, QWidget* parent = nullptr);
    explicit SceneWidget(QWidget* parent = nullptr);

    void setMachine(shared_ptr<Machine> machine, ResourceBar* resources);

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
    ResourceBar* resourcesBar = nullptr;

    QPushButton* buttonZoomIn = nullptr;
    QPushButton* buttonZoomOut = nullptr;

    bool movingScene = false;
};

#endif // SCENEWIDGET_H
