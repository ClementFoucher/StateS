/*
 * Copyright © 2014-2022 Clément Foucher
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
#include <QFormLayout>
#include <QCoreApplication>
#include <QMouseEvent>

// StateS classes
#include "labelwithclickevent.h"


CheckBoxHtml::CheckBoxHtml(const QString& text, QWidget* parent) :
    QWidget(parent)
{
	QFormLayout* layout = new QFormLayout(this);

	this->checkBox = new QCheckBox(this);
	connect(this->checkBox, &QCheckBox::toggled, this, &CheckBoxHtml::toggled);

	this->label = new LabelWithClickEvent(text, this);
	connect(this->label, &LabelWithClickEvent::clicked, this, &CheckBoxHtml::labelClicked);

	this->label->setTextFormat(Qt::RichText);

	layout->addRow(this->label, this->checkBox);
}

void CheckBoxHtml::labelClicked(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		if (e->type() == QEvent::Type::MouseButtonRelease)
		{
			this->checkBox->toggle();
		}
	}
}

void CheckBoxHtml::setText(QString newText)
{
	this->label->setText(newText);
}

void CheckBoxHtml::setChecked(bool check)
{
	this->checkBox->setChecked(check);
}

bool CheckBoxHtml::isChecked()
{
	return this->checkBox->isChecked();
}

