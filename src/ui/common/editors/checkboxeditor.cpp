/*
 * Copyright © 2025 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "checkboxeditor.h"

// Qt classes
#include <QHBoxLayout>
#include <QCheckBox>


CheckBoxEditor::CheckBoxEditor(QWidget* parent) :
	QWidget(parent)
{
	auto editorLayout = new QHBoxLayout(this);
	editorLayout->setAlignment(Qt::AlignCenter);
	editorLayout->setContentsMargins(0,0,0,0);

	this->checkBox = new QCheckBox();
	this->checkBox->setStyleSheet("QCheckBox::indicator:checked {background-color : white;}");
	this->checkBox->setStyleSheet("QCheckBox::indicator:unchecked {background-color : white;}");
	editorLayout->addWidget(this->checkBox);

	connect(this->checkBox, &QCheckBox::checkStateChanged, this, &CheckBoxEditor::checkedStateChangedEventHandler);
}

void CheckBoxEditor::setChecked(bool checked)
{
	this->inhibitEvents = true;
	if (checked == true)
	{
		this->checkBox->setCheckState(Qt::Checked);
	}
	else // (checked == false)
	{
		this->checkBox->setCheckState(Qt::Unchecked);
	}
	this->inhibitEvents = false;
}

bool CheckBoxEditor::getChecked() const
{
	return (this->checkBox->checkState() == Qt::Checked) ? true : false;
}

void CheckBoxEditor::checkedStateChangedEventHandler(Qt::CheckState)
{
	if (this->inhibitEvents == false)
	{
		emit this->checkedStateChangedEvent(this);
	}
}
