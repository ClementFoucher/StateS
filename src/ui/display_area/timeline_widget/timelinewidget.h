/*
 * Copyright © 2014-2023 Clément Foucher
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

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

// Parent
#include <QMainWindow>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QWidget;
class QToolBar;
class QAction;


class TimelineWidget : public QMainWindow
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit TimelineWidget(QWidget* parent = nullptr);

	/////
	// Object functions
signals:
	void detachTimelineEvent(bool detach);
	void outputDelayChangedEvent(uint newDelay);

protected:
	virtual void closeEvent     (QCloseEvent*) override;
	virtual void mousePressEvent(QMouseEvent*) override;
	virtual void mouseMoveEvent (QMouseEvent*) override;
	virtual void paintEvent     (QPaintEvent*) override;

private slots:
	void exportToPDF();
	void setMeFree();
	void bindMe();
	void delayOutputOptionTriggered(bool activated);

	/////
	// Object variables
private:
	QAction* actionDetach = nullptr;

	QToolBar* toolBar = nullptr;

	uint separatorPosition = 0;

};

#endif // TIMELINEWIDGET_H
