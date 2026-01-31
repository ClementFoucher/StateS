/*
 * Copyright © 2014-2026 Clément Foucher
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
#include <QBoxLayout>
#include <QPushButton>
#include <QIntValidator>
#include <QMouseEvent>
#include <QKeyEvent>

// StateS classes
#include "coloredlineeditor.h"


LineWithUpDownButtonsEditor::LineWithUpDownButtonsEditor(int min, int max, const QString& text, QWidget* parent) :
	QWidget(parent)
{
	//
	// Buttons
	auto buttonsLayout = new QVBoxLayout();
	buttonsLayout->setContentsMargins(QMargins());
	buttonsLayout->setSpacing(0);

	auto buttonUp   = new QPushButton("+");
	auto buttonDown = new QPushButton("-");

	buttonUp  ->setMinimumSize(QSize(20, 20));
	buttonDown->setMinimumSize(QSize(20, 20));

	buttonUp  ->setMaximumSize(QSize(20, 20));
	buttonDown->setMaximumSize(QSize(20, 20));

	buttonsLayout->addStretch();
	buttonsLayout->addWidget(buttonUp);
	buttonsLayout->addWidget(buttonDown);
	buttonsLayout->addStretch();

	//
	// Line editor
	this->validator = new QIntValidator(min, max);
	QString initialText = text;
	int pos;
	if (this->validator->validate(initialText, pos) != QValidator::State::Invalid)
	{
		this->lineEdit = new ColoredLineEditor(text);
	}
	else
	{
		this->lineEdit = new ColoredLineEditor();
	}

	this->lineEdit->setValidator(validator);
	this->lineEdit->setMaximumWidth(50);

	//
	// Build complete rendering
	auto mainLayout = new QHBoxLayout(this);

	mainLayout->addLayout(buttonsLayout);
	buttonsLayout->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

	mainLayout->addWidget(this->lineEdit);
	this->lineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

	//
	// Connect signals
	connect(buttonUp,   &QPushButton::clicked, this, &LineWithUpDownButtonsEditor::up);
	connect(buttonDown, &QPushButton::clicked, this, &LineWithUpDownButtonsEditor::down);

	connect(this->lineEdit, &ColoredLineEditor::textChanged, this, &LineWithUpDownButtonsEditor::textUpdatedByUserEventHandler);
}

int LineWithUpDownButtonsEditor::getValue() const
{
	auto currentText = this->lineEdit->text();
	return currentText.toInt();
}

void LineWithUpDownButtonsEditor::setMinValue(int min)
{
	this->validator->setBottom(min);
}

void LineWithUpDownButtonsEditor::setMaxValue(int max)
{
	this->validator->setTop(max);
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

void LineWithUpDownButtonsEditor::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Up)
	{
		this->up();
	}
	else if (event->key() == Qt::Key::Key_Down)
	{
		this->down();
	}
	else
	{
		QWidget::keyPressEvent(event);
	}
}

void LineWithUpDownButtonsEditor::keyReleaseEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Up)
	{
		// KeyPressEvent handled, so we have to handle keyReleaseEvent too.
	}
	else if (event->key() == Qt::Key::Key_Down)
	{
		// KeyPressEvent handled, so we have to handle keyReleaseEvent too.
	}
	else
	{
		QWidget::keyReleaseEvent(event);
	}
}

void LineWithUpDownButtonsEditor::focusInEvent(QFocusEvent*)
{
	if (this->lineEdit == nullptr) return;

	this->lineEdit->setFocus();
}

void LineWithUpDownButtonsEditor::textUpdatedByUserEventHandler(const QString& newText)
{
	int pos;
	QString text2 = newText;
	auto valid = this->validator->validate(text2, pos);

	if (valid == QValidator::State::Acceptable)
	{
		this->lineEdit->setErroneous(false);

		emit this->valueChanged(newText.toInt());
	}
	else
	{
		this->lineEdit->setErroneous(true);
	}
}

void LineWithUpDownButtonsEditor::up()
{
	int pos;
	QString currentText = this->lineEdit->text();

	if (this->validator->validate(currentText, pos) != QValidator::State::Acceptable)
	{
		this->lineEdit->setText(QString::number(this->validator->bottom()));
	}
	else
	{
		int currentValue = this->lineEdit->text().toInt();
		currentValue++;

		QString newText = QString::number(currentValue);
		if (this->validator->validate(newText, pos) == QValidator::State::Acceptable)
		{
			this->lineEdit->setText(newText);
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
	}
	else
	{
		int currentValue = this->lineEdit->text().toInt();
		currentValue--;

		QString newText = QString::number(currentValue);
		if (this->validator->validate(newText, pos) == QValidator::State::Acceptable)
		{
			this->lineEdit->setText(QString::number(currentValue));
		}
	}
}
