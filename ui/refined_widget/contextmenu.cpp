/*
 * Copyright © 2014-2016 Clément Foucher
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
#include "contextmenu.h"

// Qt classes
#include <QHelpEvent>
#include <QLabel>
#include <QToolTip>
#include <QWidgetAction>


const QString ContextMenu::errorStyle("background-color: lightgrey; border: 3px solid red; color: red");
const QString ContextMenu::listStyle("background-color: lightgrey; border: 3px double");

ContextMenu::ContextMenu(QWidget* parent) :
    QMenu(parent)
{
	connect(this, &ContextMenu::aboutToHide, this, &ContextMenu::deleteLater);
	this->setStyleSheet( QString( "border: 1px solid"));
}

void ContextMenu::addTitle(const QString& titleText)
{
	QLabel* title = new QLabel("<b>" + titleText + "</b>");
	title->setAlignment(Qt::AlignCenter);
	title->setMinimumHeight(50);
	title->setMargin(10);

	QWidgetAction* a = new QWidgetAction(this);
	a->setDefaultWidget(title);
	this->addAction(a);
}

void ContextMenu::addSubTitle(const QString& titleText)
{
	QLabel* subtitle = new QLabel(titleText);
	subtitle->setAlignment(Qt::AlignCenter);
	subtitle->setMinimumHeight(40);
	subtitle->setMargin(10);

	QWidgetAction* a = new QWidgetAction(this);
	a->setDefaultWidget(subtitle);
	this->addAction(a);
}

void ContextMenu::setListStyle()
{
	this->setStyleSheet(listStyle);
}

// Code found in Qt FAQ to add tool tip support to menus...
// Why not default?
bool ContextMenu::event(QEvent* e)
{
	if (this->activeAction() != nullptr)
	{
		if (this->activeAction()->toolTip().isEmpty())
		{
			const QHelpEvent* helpEvent = dynamic_cast <QHelpEvent*>(e);

			if ( (helpEvent != nullptr) && (helpEvent->type() == QEvent::ToolTip) )
			{
				QToolTip::showText(helpEvent->globalPos(), activeAction()->toolTip());
			}
			else
			{
				QToolTip::hideText();
			}
		}
	}

	return QMenu::event(e);
}

ContextMenu* ContextMenu::createErrorMenu(const QString& text, QWidget* parent)
{
	ContextMenu* newMenu = new ContextMenu(parent);
	newMenu->setStyleSheet("");

	newMenu->addAction(text);

	newMenu[0].setStyleSheet(errorStyle);

	return newMenu;
}
