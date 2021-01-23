/*
 * Copyright © 2014-2020 Clément Foucher
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

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H

// Parent
#include <QMainWindow>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QTabWidget;

// StateS classes
#include "machine.h"
class TimelineWidget;
class MainToolBar;
class MachineEditorWidget;


/**
 * @brief The DisplayArea class handles the central display.
 * Usually, it just displays the editor area, but more widgets
 * can be added that will be displayed as tabs. E.g. when in
 * simulation mode, the timeline is added as a tab.
 * This class also owns the main tool bar for correct placement.
 */
class DisplayArea : public QMainWindow
{
	Q_OBJECT

public:
	explicit DisplayArea(QWidget* parent = nullptr);

	void setToolBar  (QToolBar* toolbar);
	void addWidget   (QWidget* widget, QString title);
	void removeWidget(QWidget* widget);

private:
	QList<tuple<QString, QWidget*>> widgets;
	QToolBar* toolbar = nullptr;
	QTabWidget* tabWidget = nullptr;
};

#endif // DISPLAYAREA_H
