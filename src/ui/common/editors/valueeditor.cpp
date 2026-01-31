/*
 * Copyright © 2025-2026 Clément Foucher
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
#include "valueeditor.h"

// Qt classes
#include <QHBoxLayout>
#include <QRegularExpressionValidator>
#include <QKeyEvent>

// StateS classes
#include "logicvalue.h"
#include "coloredlineeditor.h"


ValueEditor::ValueEditor(QWidget* parent) :
	QWidget(parent)
{
	auto sizePolicy = this->sizePolicy();
	sizePolicy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	this->setSizePolicy(sizePolicy);

	this->lineEdit = new ColoredLineEditor();
	connect(this->lineEdit, &ColoredLineEditor::editingFinished, this, &ValueEditor::valueChangedEvent);

	auto editorLayout = new QHBoxLayout(this);
	editorLayout->setContentsMargins(0, 0, 0, 0);
	editorLayout->addWidget(this->lineEdit);
}

void ValueEditor::setFocusOnShow(bool autoFocusOnNextShow)
{
	this->autoFocusOnNextShow = autoFocusOnNextShow;
}

void ValueEditor::setBitVectorValue(LogicValue value, uint size)
{
	this->bitVectorSize = size;

	QRegularExpression re;
	if (size != 0)
	{
		re = QRegularExpression("[01]{0," + QString::number(size) + "}");
	}
	else
	{
		re = QRegularExpression("[01]*");
	}

	this->lineEdit->setValidator(new QRegularExpressionValidator(re));
	this->lineEdit->setText(value.toString());
}

LogicValue ValueEditor::getBitVectorValue() const
{
	auto currentValue = LogicValue::fromString(this->lineEdit->text());

	if (this->bitVectorSize != 0)
	{
		if (currentValue.getSize() != this->bitVectorSize)
		{
			currentValue.resize(this->bitVectorSize);
		}
	}

	return currentValue;
}

void ValueEditor::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	if (this->autoFocusOnNextShow == true)
	{
		this->setFocus();
		this->autoFocusOnNextShow = false;
	}
}

void ValueEditor::focusInEvent(QFocusEvent* event)
{
	QWidget::focusInEvent(event);

	this->lineEdit->setFocus();
	this->lineEdit->selectAll();
}

void ValueEditor::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Escape)
	{
		emit this->cancelEditEvent();
	}
	else
	{
		QWidget::keyPressEvent(event);
	}
}
