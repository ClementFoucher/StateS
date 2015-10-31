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
#include "dynamiclineedit.h"

// Qt classes
#include <QKeyEvent>


DynamicLineEdit::DynamicLineEdit(const QString& content, bool selfManaged, shared_ptr<QValidator> validator, QWidget* parent) :
    QLineEdit(content, parent)
{
    this->selfManaged = selfManaged;

    if (this->selfManaged)
        connect(this, &QLineEdit::editingFinished, this, &DynamicLineEdit::userValidatedEventHandler);

    if (validator != nullptr)
        this->setValidator(validator.get());
}

void DynamicLineEdit::userValidatedEventHandler()
{
    // For self-managed lines: go back to normal mode on edit end.
    this->resetView();

    // Refused mode will be triggered if text is incorrect
    emit newTextAvailableEvent(this->text());
}

void DynamicLineEdit::markAsErroneous()
{
    this->erroneous = true;
    this->setStyleSheet( QString( "background-color: red") );

    // When erroneous, force focus to continue edit
    this->setFocus();
}

void DynamicLineEdit::resetView()
{
    this->erroneous = false;
    this->setModified(false);
    if (this->selfManaged)
        disconnect(this, &QLineEdit::editingFinished, this, &DynamicLineEdit::userValidatedEventHandler);
    this->clearFocus();
    if (this->selfManaged)
        connect(this, &QLineEdit::editingFinished, this, &DynamicLineEdit::userValidatedEventHandler);

    this->setStyleSheet( QString( "background-color: white"));
}

void DynamicLineEdit::focusInEvent(QFocusEvent* event)
{
    if (!erroneous)
        this->setStyleSheet( QString( "background-color: yellow"));

    QLineEdit::focusInEvent(event);
}

void DynamicLineEdit::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key::Key_Escape)
    {
        emit userCancelEvent();
    }
    else
    {
        QLineEdit::keyPressEvent(event);
    }
}
