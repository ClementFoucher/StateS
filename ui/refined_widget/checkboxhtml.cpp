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
#include "checkboxhtml.h"

// Qt classes
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>


CheckBoxHtml::CheckBoxHtml(const QString& text, Qt::AlignmentFlag boxAlign, bool allowLink, QWidget* parent) :
    QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    this->checkBox = new QCheckBox();
    connect(this->checkBox, &QCheckBox::clicked, this, &CheckBoxHtml::clicked);

    this->label = new QLabel(text);
    this->label->setTextFormat(Qt::RichText);
    if (allowLink)
    {
        this->label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        this->label->setOpenExternalLinks(true);
    }

    if (boxAlign == Qt::AlignmentFlag::AlignLeft)
    {
        layout->addWidget(this->checkBox);
        layout->addWidget(this->label);
    }
    else
    {
        layout->addWidget(this->label);
        layout->addWidget(this->checkBox);
    }
}

void CheckBoxHtml::mousePressEvent(QMouseEvent*)
{
    // Ideally, should only transmit events to the check box so that
    // it reacts on the whole widget with the exact same behavior
    this->checkBox->toggle();
}

void CheckBoxHtml::setText(QString newText)
{
    this->label->setText(newText);
}

bool CheckBoxHtml::isChecked()
{
    return this->checkBox->isChecked();
}

