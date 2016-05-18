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
#ifndef TOOLBAR_H
#define TOOLBAR_H

// Parent class
#include <QToolBar>


class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit ToolBar(QWidget* parent = nullptr);

    void setSaveAsActionEnabled (bool enable);
    void setSaveActionEnabled   (bool enable);
    void setExportActionsEnabled(bool enable);

signals:
    void saveAsRequestedEvent     ();
    void saveRequestedEvent       ();
    void loadRequestedEvent       ();
    void newMachineRequestedEvent ();
    void exportImageRequestedEvent();
    void exportHdlRequestedEvent  ();

private:
    QAction* actionSaveAs      = nullptr;
    QAction* actionSave        = nullptr;
    QAction* actionLoad        = nullptr;
    QAction* actionNewFsm      = nullptr;
    //QAction* actionClear      = nullptr;
    QAction* actionExportImage = nullptr;
    QAction* actionExportHdl   = nullptr;
};

#endif // TOOLBAR_H
