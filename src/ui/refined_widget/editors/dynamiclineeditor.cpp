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

// Qt classes
#include <QKeyEvent>


//
// Static elements
//

const QString DynamicLineEditor::editStyle  = QString("background-color: yellow; color: black;");
const QString DynamicLineEditor::errorStyle = QString("background-color: red; color: black;");


//
// Class object definition
//

DynamicLineEditor::DynamicLineEditor(const QString& content, bool selfManaged, QValidator* validator, QWidget* parent) :
    QLineEdit(content, parent)
{
	this->selfManaged = selfManaged;

	if (this->selfManaged)
	{
		connect(this, &QLineEdit::editingFinished, this, &DynamicLineEditor::userValidatedEventHandler);
	}

	if (validator != nullptr)
	{
		this->setValidator(validator);
	}
}

DynamicLineEditor::DynamicLineEditor(const QString& content, bool selfManaged, QWidget* parent) :
	DynamicLineEditor(content, selfManaged, nullptr, parent)
{

}

void DynamicLineEditor::userValidatedEventHandler()
{
	// For self-managed lines: go back to normal mode on edit end.
	this->resetView();

	// Refused mode will be triggered if text is incorrect
	emit newTextAvailableEvent(this->text());
}

void DynamicLineEditor::markAsErroneous()
{
	this->erroneous = true;
	this->setStyleSheet(DynamicLineEditor::errorStyle);

	// When erroneous, force focus to continue edit
	this->setFocus();
}

void DynamicLineEditor::resetView()
{
	this->erroneous = false;
	this->setModified(false);
	if (this->selfManaged)
	{
		disconnect(this, &QLineEdit::editingFinished, this, &DynamicLineEditor::userValidatedEventHandler);
	}
	this->clearFocus();
	if (this->selfManaged)
	{
		connect(this, &QLineEdit::editingFinished, this, &DynamicLineEditor::userValidatedEventHandler);
	}

	this->setStyleSheet( QString() );
}

void DynamicLineEditor::focusInEvent(QFocusEvent* event)
{
	if (!erroneous)
	{
		this->setStyleSheet(DynamicLineEditor::editStyle);
	}

	QLineEdit::focusInEvent(event);
}

void DynamicLineEditor::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key::Key_Escape)
	{
		emit userCancelEvent();
		transmitEvent = false;
	}
	else if (event->key() == Qt::Key::Key_Up)
	{
		emit upKeyPressed();
	}
	else if (event->key() == Qt::Key::Key_Down)
	{
		emit downKeyPressed();
	}

	if (transmitEvent)
	{
		QLineEdit::keyPressEvent(event);
	}
}

void DynamicLineEditor::keyReleaseEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key::Key_Escape)
	{
		transmitEvent = false;
	}

	if (transmitEvent == true)
	{
		QLineEdit::keyReleaseEvent(event);
	}
}
