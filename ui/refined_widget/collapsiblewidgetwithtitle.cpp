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
    connect(this->buttonCollapse, &QPushButton::toggled, this, &CollapsibleWidgetWithTitle::updateVisibility);

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
    if (title != QString::null)
        this->title->setText("<b>" + title + "</b>");
    else
    {
        this->title->setText("<b><i>(" + tr("no content") + ")</i></b>");
        this->title->setEnabled(false);
    }

    if (this->content != nullptr)
    {
        this->layout->removeWidget(this->content);
        if (deletePreviousContent)
            delete this->content;
    }

    this->content = content;

    if (this->content != nullptr)
    {
        // Make sure added content is in correct display state wrt. current button state
        this->content->setVisible(this->buttonCollapse->isChecked());
        this->layout->addWidget(this->content, 1, 0, 1, 4);

        this->buttonCollapse->setEnabled(true);
        if (title != QString::null)
            this->title->setEnabled(true);
    }
    else
    {
        this->title->setEnabled(false);
        this->buttonCollapse->setEnabled(false);
    }
}

void CollapsibleWidgetWithTitle::updateVisibility(bool show)
{
    if (show)
        this->extend();
    else
        this->collapse();
}

void CollapsibleWidgetWithTitle::collapse()
{
    this->buttonCollapse->setText(tr("Show"));

    if (this->content != nullptr)
    {
        this->content->setVisible(false);
    }
}

void CollapsibleWidgetWithTitle::extend()
{
    this->buttonCollapse->setText(tr("Hide"));

    if (this->content != nullptr)
    {
        this->content->setVisible(true);
    }
}

