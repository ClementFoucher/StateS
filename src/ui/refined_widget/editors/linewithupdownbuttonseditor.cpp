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
#include "linewithupdownbuttonseditor.h"

// Qt classes
#include <QHBoxLayout>
#include <QPushButton>
#include <QIntValidator>
#include <QMouseEvent>

// StateS classes
#include "selfmanageddynamiclineeditor.h"


LineWithUpDownButtonsEditor::LineWithUpDownButtonsEditor(int min, int max, const QString& text, QWidget* parent) :
    QWidget(parent)
{
	QWidget* buttonWidget = new QWidget(this);
	buttonWidget->setMinimumSize(QSize(20, 40));

	QPushButton* buttonUp   = new QPushButton("+", buttonWidget);
	QPushButton* buttonDown = new QPushButton("-", buttonWidget);

	buttonUp->resize(QSize(20, 20));
	buttonDown->resize(QSize(20, 20));

	buttonUp->move(0, 0);
	buttonDown->move(0, 20);


	this->validator = new QIntValidator(min, max);
	QString initialText = text;
	int pos;
	if (this->validator->validate(initialText, pos) != QValidator::State::Invalid)
	{
		this->lineEdit = new SelfManagedDynamicLineEditor(text, this);
	}
	else
	{
		this->lineEdit = new SelfManagedDynamicLineEditor(this);
	}

	this->lineEdit->setValidator(validator);
	this->lineEdit->setMaximumWidth(50);

	connect(buttonUp,   &QPushButton::clicked, this, &LineWithUpDownButtonsEditor::up);
	connect(buttonDown, &QPushButton::clicked, this, &LineWithUpDownButtonsEditor::down);

	connect(this->lineEdit, &SelfManagedDynamicLineEditor::textEdited,     this, &LineWithUpDownButtonsEditor::textUpdatedByUserEventHandler);
	connect(this->lineEdit, &SelfManagedDynamicLineEditor::upKeyPressed,   this, &LineWithUpDownButtonsEditor::up);
	connect(this->lineEdit, &SelfManagedDynamicLineEditor::downKeyPressed, this, &LineWithUpDownButtonsEditor::down);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(buttonWidget);
	layout->addWidget(this->lineEdit);
}

void LineWithUpDownButtonsEditor::updateContent(int min, int max, const QString& text)
{
	delete this->validator;
	this->validator = new QIntValidator(min, max);
	this->lineEdit->setValidator(this->validator);

	QString initialText = text;
	int pos;
	if (this->validator->validate(initialText, pos) != QValidator::State::Invalid)
	{
		this->lineEdit->setText(text);
	}
	else
	{
		this->lineEdit->setText(QString());
	}
}

void LineWithUpDownButtonsEditor::edit()
{
	this->lineEdit->setFocus();
}

void LineWithUpDownButtonsEditor::wheelEvent(QWheelEvent* event)
{
	if(event->angleDelta().y() > 0)
	{
		this->up();
	}
	else
	{
		this->down();
	}
}

void LineWithUpDownButtonsEditor::up()
{
	int pos;
	QString currentText = this->lineEdit->text();

	if (this->validator->validate(currentText, pos) != QValidator::State::Acceptable)
	{
		this->lineEdit->setText(QString::number(this->validator->bottom()));
		emit valueChanged(this->validator->bottom());
	}
	else
	{
		int currentValue = this->lineEdit->text().toInt();
		currentValue++;

		QString newText = QString::number(currentValue);
		if (this->validator->validate(newText, pos) == QValidator::State::Acceptable)
		{
			this->lineEdit->setText(newText);
			emit valueChanged(currentValue);
		}
	}
}

void LineWithUpDownButtonsEditor::down()
{
	int pos;
	QString currentText = this->lineEdit->text();

	if (this->validator->validate(currentText, pos) != QValidator::State::Acceptable)
	{
		this->lineEdit->setText(QString::number(this->validator->top()));
		emit valueChanged(this->validator->top());
	}
	else
	{
		int currentValue = this->lineEdit->text().toInt();
		currentValue--;

		QString newText = QString::number(currentValue);
		if (this->validator->validate(newText, pos) == QValidator::State::Acceptable)
		{
			this->lineEdit->setText(QString::number(currentValue));
			emit valueChanged(currentValue);
		}
	}
}

void LineWithUpDownButtonsEditor::textUpdatedByUserEventHandler(const QString& newText)
{
	emit valueChanged(newText.toInt());
}

