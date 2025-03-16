/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "dynamiclineeditor.h"


//
// Static elements
//

const QString DynamicLineEditor::editStyle  = QString("QLineEdit {background-color: yellow; color: black;}");
const QString DynamicLineEditor::errorStyle = QString("QLineEdit {background-color: red; color: black;}");

//
// Class object definition
//

DynamicLineEditor::DynamicLineEditor(const QString& content, QWidget* parent) :
	QLineEdit(content, parent)
{
}

DynamicLineEditor::DynamicLineEditor(QWidget* parent) :
	QLineEdit(parent)
{
}

void DynamicLineEditor::setErroneous(bool erroneous)
{
	this->erroneous = erroneous;

	if (this->erroneous == true)
	{
		this->setStyleSheet(DynamicLineEditor::errorStyle);

		// When erroneous, force focus to continue edit
		this->setFocus();
	}
	else
	{
		this->setStyleSheet(QString());
	}
}

bool DynamicLineEditor::getIsErroneous() const
{
	return this->erroneous;
}

void DynamicLineEditor::focusInEvent(QFocusEvent* event)
{
	if (this->erroneous == false)
	{
		this->setStyleSheet(DynamicLineEditor::editStyle);
	}

	QLineEdit::focusInEvent(event);
}

void DynamicLineEditor::focusOutEvent(QFocusEvent* event)
{
	if (this->erroneous == false)
	{
		this->setStyleSheet(QString());
	}

	QLineEdit::focusOutEvent(event);
}
