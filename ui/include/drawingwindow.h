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

#ifndef DRAWINGWINDOW_H
#define DRAWINGWINDOW_H

#include <QMainWindow>

class Machine;
class CentralWidget;

namespace Ui {
class DrawingWindow;
}

class DrawingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DrawingWindow(Machine* machine = 0);
    ~DrawingWindow();

    void setMachine(Machine* machine);

public slots: // Debug
    void clearMachine();
    void newMachine();

protected slots:
    void closeEvent(QCloseEvent*) override;

private slots:
    void on_actionNew_triggered();

    void on_actionClear_triggered();

    void on_actionExport_triggered();

    void on_actionSave_triggered();

    void on_actionLoad_triggered();

private:
    Ui::DrawingWindow* ui = nullptr;

    CentralWidget* centralWidget = nullptr;

    // For debug only
    Machine* machine = nullptr;

};

#endif // DRAWINGWINDOW_H
