/*
 * Copyright © 2014-2017 Clément Foucher
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
#include "collapsiblewidgetwithtitle.h"

// Qt classes
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>


CollapsibleWidgetWithTitle::CollapsibleWidgetWithTitle(const QString& title, QWidget* content, QWidget* parent) :
    QFrame(parent)
{
	this->layout         = new QGridLayout(this);
	this->buttonCollapse = new QPushButton();
	this->title          = new QLabel();

	// Define style

	this->setFrameStyle(QFrame::Box | QFrame::Raised);

	this->buttonCollapse->setCheckable(true);
	this->buttonCollapse->setChecked(true);
	connect(this->buttonCollapse, &QPushButton::toggled, this, &CollapsibleWidgetWithTitle::buttonCollapseToggledEventHander);

	this->title->setAlignment(Qt::AlignCenter);

	// Add content

	this->layout->addWidget(this->title,          0, 0, 1, 3);
	this->layout->addWidget(this->buttonCollapse, 0, 3, 1, 1);

	// Update view

	this->setContent(title, content);
	this->extend();
}

void CollapsibleWidgetWithTitle::setContent(const QString& title, QWidget* content, bool deletePreviousContent)
{
	if (this->content != nullptr)
	{
		if (deletePreviousContent)
		{
			delete this->content;
		}
		else
		{
			this->content->setVisible(false);
			this->layout->removeWidget(this->content);
		}
	}

	this->content = content;

	if (this->content != nullptr)
	{
		// Make sure added content is in correct display state wrt. current button state
		this->content->setVisible(this->buttonCollapse->isChecked());
		this->layout->addWidget(this->content, 1, 0, 1, 4);

		this->buttonCollapse->setEnabled(true);

		if (title != QString::null)
		{
			this->title->setText("<b>" + title + "</b>");
			this->title->setEnabled(true);
		}
		else
		{
			this->title->setText("<b><i>(" + tr("no title") + ")</i></b>");
			this->title->setEnabled(false);
		}
	}
	else
	{
		this->title->setText("<b><i>(" + tr("no content") + ")</i></b>");
		this->title->setEnabled(false);
		this->buttonCollapse->setEnabled(false);
	}
}

void CollapsibleWidgetWithTitle::setContent(const QString& title, const QString& textContent, bool deletePreviousContent)
{
	QLabel* newContent = new QLabel(textContent, this);
	newContent->setAlignment(Qt::AlignCenter);
	newContent->setWordWrap(true);

	this->setContent(title, newContent, deletePreviousContent);
}

void CollapsibleWidgetWithTitle::setCollapsed(bool collapse)
{
	if (collapse)
	{
		this->collapse();
	}
	else
	{
		this->extend();
	}
}

bool CollapsibleWidgetWithTitle::getCollapsed()
{
	return !(this->buttonCollapse->isChecked());
}

void CollapsibleWidgetWithTitle::buttonCollapseToggledEventHander(bool buttonPushed)
{
	this->setCollapsed(!buttonPushed);
}

void CollapsibleWidgetWithTitle::collapse()
{
	this->buttonCollapse->setText(tr("Show"));
	this->buttonCollapse->setChecked(false);

	if (this->content != nullptr)
	{
		this->content->setVisible(false);
	}
}

void CollapsibleWidgetWithTitle::extend()
{
	this->buttonCollapse->setText(tr("Hide"));
	this->buttonCollapse->setChecked(true);

	if (this->content != nullptr)
	{
		this->content->setVisible(true);
	}
}

