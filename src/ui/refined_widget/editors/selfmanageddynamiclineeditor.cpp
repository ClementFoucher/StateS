/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "selfmanageddynamiclineeditor.h"

// Qt classes
#include <QKeyEvent>


SelfManagedDynamicLineEditor::SelfManagedDynamicLineEditor(const QString& content, QWidget* parent) :
    DynamicLineEditor(content, parent)
{
	// The widget will automatically reset
	// its visual style and yield focus on user validation.
	// Its newTextAvailableEvent signal should be
	// connected instead of editingFinished.
	connect(this, &QLineEdit::editingFinished, this, &SelfManagedDynamicLineEditor::userValidatedEventHandler);
}

SelfManagedDynamicLineEditor::SelfManagedDynamicLineEditor(QWidget* parent) :
    SelfManagedDynamicLineEditor(nullptr, parent)
{
}

void SelfManagedDynamicLineEditor::userValidatedEventHandler()
{
	// Go back to normal mode on edit end.
	this->resetView();

	// Erroneous mode should be triggered if text is incorrect
	emit newTextAvailableEvent(this->text());
}

void SelfManagedDynamicLineEditor::resetView()
{
	this->erroneous = false;
	this->setModified(false);
	disconnect(this, &QLineEdit::editingFinished, this, &SelfManagedDynamicLineEditor::userValidatedEventHandler);
	this->clearFocus();
	connect(this, &QLineEdit::editingFinished, this, &SelfManagedDynamicLineEditor::userValidatedEventHandler);

	this->setStyleSheet( QString() );
}

void SelfManagedDynamicLineEditor::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key::Key_Up)
	{
		emit upKeyPressed();
	}
	else if (event->key() == Qt::Key::Key_Down)
	{
		emit downKeyPressed();
	}

	if (transmitEvent == true)
	{
		DynamicLineEditor::keyPressEvent(event);
	}
}

void SelfManagedDynamicLineEditor::keyReleaseEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key::Key_Up)
	{
		transmitEvent = false;
	}
	else if (event->key() == Qt::Key::Key_Down)
	{
		transmitEvent = false;
	}

	if (transmitEvent == true)
	{
		DynamicLineEditor::keyReleaseEvent(event);
	}
}
