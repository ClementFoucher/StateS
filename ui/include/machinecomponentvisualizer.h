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

#ifndef MACHINECOMPONENTVISUALIZER_H
#define MACHINECOMPONENTVISUALIZER_H

// Parent
#include <QGraphicsView>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QGraphicsScene;
class QGraphicsView;
class QMouseEvent;

// StateS classes
class Machine;


class MachineComponentVisualizer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MachineComponentVisualizer(shared_ptr<Machine> machine, QWidget* parent = nullptr);

    shared_ptr<QGraphicsScene> getComponentVisualizationScene() const;

protected:
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;
    void mouseDoubleClickEvent(QMouseEvent* me) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void updateMachineVisualization();

private:
    weak_ptr<Machine> machine;

    shared_ptr<QGraphicsScene> scene;

    // Qwidget with parent
    QGraphicsView* view = nullptr;

    bool isMoving = false;
};

#endif // MACHINECOMPONENTVISUALIZER_H
