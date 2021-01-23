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

// Current class header
#include "displayarea.h"

// StateS classes
#include "maintoolbar.h"
#include "machineeditorwidget.h"
#include "timelinewidget.h"


DisplayArea::DisplayArea(QWidget* parent) :
    QMainWindow(parent)
{
	this->setWindowFlags(Qt::Widget);
	this->setContextMenuPolicy(Qt::NoContextMenu);
}

void DisplayArea::setToolBar(QToolBar* toolbar)
{
	if (this->toolbar == nullptr)
	{
		this->toolbar = toolbar;
		this->toolbar->setMovable(true);
		this->addToolBar(Qt::LeftToolBarArea, this->toolbar);
	}
}

void DisplayArea::addWidget(QWidget* widget, QString title)
{
	int previousWidgetCount = this->widgets.count();

	tuple<QString, QWidget*> t(title, widget);
	this->widgets.append(t);

	if (previousWidgetCount == 0)
	{
		// Only one widget: no tabs
		this->setCentralWidget(widget);
		widget->show();
	}
	else if (previousWidgetCount == 1)
	{
		// From 1 to more widgets: build tabs
		this->tabWidget = new QTabWidget(this);

		this->tabWidget->addTab(get<1>(this->widgets[0]), get<0>(this->widgets[0]));
		this->tabWidget->addTab(get<1>(this->widgets[1]), get<0>(this->widgets[1]));

		this->setCentralWidget(this->tabWidget);
	}
	else
	{
		// Tabs already existing: just add
		this->tabWidget->addTab(widget, title);
	}
}

void DisplayArea::removeWidget(QWidget* widget)
{
	for (tuple<QString, QWidget*> t : this->widgets)
	{
		QWidget* currentWidget = get<1>(t);
		if (currentWidget == widget)
		{
			this->widgets.removeAll(t);
			widget->setParent(nullptr);

			int newWidgetCount = this->widgets.count();

			if (newWidgetCount == 1)
			{
				// Remove tabs
				QWidget* soleWidget = get<1>(this->widgets.first());
				this->setCentralWidget(soleWidget);
				soleWidget->show();

				delete this->tabWidget;
				this->tabWidget = nullptr;
			}
			else if (newWidgetCount == 0)
			{
				this->setCentralWidget(nullptr);
			}

			break;
		}
	}
}
