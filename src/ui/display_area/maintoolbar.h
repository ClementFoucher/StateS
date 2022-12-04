/*
 * Copyright © 2014-2021 Clément Foucher
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

#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

// Parent class
#include <QToolBar>


class MainToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit MainToolBar(QWidget* parent = nullptr);

	void setSaveAsActionEnabled (bool enable);
	void setSaveActionEnabled   (bool enable);
	void setNewFsmActionEnabled (bool enable);
	void setExportActionsEnabled(bool enable);
	void setUndoActionEnabled   (bool enable);
	void setRedoActionEnabled   (bool enable);

	bool getUndoActionEnabled() const;
	bool getRedoActionEnabled() const;

signals:
	void saveAsRequestedEvent     ();
	void saveRequestedEvent       ();
	void loadRequestedEvent       ();
	void newMachineRequestedEvent ();
	void exportImageRequestedEvent();
	void exportHdlRequestedEvent  ();
	void undo                     ();
	void redo                     ();

private:
	QAction* actionSaveAs      = nullptr;
	QAction* actionSave        = nullptr;
	QAction* actionLoad        = nullptr;
	QAction* actionNewFsm      = nullptr;
	//QAction* actionClear       = nullptr;
	QAction* actionExportImage = nullptr;
	QAction* actionExportHdl   = nullptr;
	QAction* actionUndo        = nullptr;
	QAction* actionRedo        = nullptr;
};

#endif // MAINTOOLBAR_H
