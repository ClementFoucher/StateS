/*
 * Copyright © 2014-2026 Clément Foucher
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
#include <QLabel>
#include <QWidgetAction>
#include <QVBoxLayout>


const QString ContextMenu::defaultStyle("QMenu {border: 1px solid}");
const QString ContextMenu::errorStyle("QMenu {background-color: lightgrey; border: 3px solid red; color: red}");
const QString ContextMenu::listStyle("QMenu {border: 3px double}");


ContextMenu* ContextMenu::createErrorMenu(const QString& text, QWidget* parent)
{
	auto textList = QStringList();
	textList.append(text);

	return ContextMenu::createErrorMenu(textList, parent);
}

ContextMenu* ContextMenu::createErrorMenu(const QStringList& texts, QWidget* parent)
{
	ContextMenu* newMenu = new ContextMenu(parent);
	newMenu->setStyleSheet(errorStyle);

	for (auto& actionText : texts)
	{
		QWidgetAction* a = new QWidgetAction(newMenu);
		a->setText(actionText);
		newMenu->addAction(a);
	}

	return newMenu;
}

ContextMenu::ContextMenu(QWidget* parent) :
	QMenu(parent)
{
	connect(this, &ContextMenu::aboutToHide, this, &ContextMenu::deleteLater);
	this->setStyleSheet(defaultStyle);
	this->setToolTipsVisible(true);
}

void ContextMenu::addTitle(const QString& titleText, const QStringList& subtitlesText)
{
	auto action = new QWidgetAction(this);

	auto title = new QLabel("<b>" + titleText + "</b>");
	title->setAlignment(Qt::AlignCenter);
	title->setStyleSheet("QLabel {margin: 1em}");

	if (subtitlesText.isEmpty() == true)
	{
		title->setMinimumHeight(50);
		action->setDefaultWidget(title);
	}
	else
	{
		title->setMinimumHeight(30);

		auto titleContainer = new QWidget();
		auto titleLayout = new QVBoxLayout(titleContainer);
		titleLayout->addWidget(title);

		for (auto& subtitleText : subtitlesText)
		{
			auto subtitle = new QLabel(subtitleText);
			subtitle->setAlignment(Qt::AlignCenter);
			subtitle->setMinimumHeight(10);
			titleLayout->addWidget(subtitle);
		}

		action->setDefaultWidget(titleContainer);
	}

	this->addAction(action);
	this->addSeparator();
}

void ContextMenu::setListStyle()
{
	this->setStyleSheet(listStyle);
}
