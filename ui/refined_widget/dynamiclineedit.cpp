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


DynamicLineEdit::DynamicLineEdit(const QString& content, bool beginAsRefused, QValidator* validator, QWidget* parent) :
    QLineEdit(content, parent)
{
    if (validator != nullptr)
        this->setValidator(validator);

    if (beginAsRefused)
        refuseText();
}

void DynamicLineEdit::userValidated()
{
    disconnect(this, &QLineEdit::editingFinished, this, &DynamicLineEdit::userValidated);

    this->setStyleSheet( QString( "background-color: white"));
    this->setModified(false);
    this->clearFocus();

    this->textRefused = false;

    emit newTextAvailable(this->text());
}

void DynamicLineEdit::refuseText()
{
    this->textRefused = true;
    this->setFocus();
}

void DynamicLineEdit::focusInEvent(QFocusEvent* event)
{
    if (!textRefused)
        this->setStyleSheet( QString( "background-color: yellow"));
    else
        this->setStyleSheet( QString( "background-color: red"));

    connect(this, &QLineEdit::editingFinished, this, &DynamicLineEdit::userValidated);

    emit QLineEdit::focusInEvent(event);
}

void DynamicLineEdit::focusOutEvent(QFocusEvent* event)
{
    if (this->isModified())
        userValidated();

    emit QLineEdit::focusOutEvent(event);
}


