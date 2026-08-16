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

// Qt
#include <QHBoxLayout>
#include <QRegularExpressionValidator>
#include <QKeyEvent>

// StateS
#include "coloredlineeditor.h"
#include "discreetcombobox.h"


ValueEditor::ValueEditor(QWidget* parent) :
	QWidget(parent)
{
	auto sizePolicy = this->sizePolicy();
	sizePolicy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	this->setSizePolicy(sizePolicy);

	auto editorLayout = new QHBoxLayout(this);
	editorLayout->setContentsMargins(0, 0, 0, 0);
}

void ValueEditor::setMachineValue(MachineValue value)
{
	auto newValueType = value.getType();
	if (newValueType == MachineValue::Type_t::nullType) return;


	if (newValueType != this->valueType)
	{
		delete this->comboBox;
		delete this->lineEdit;

		this->comboBox = nullptr;
		this->lineEdit = nullptr;
	}

	this->valueType = newValueType;

	switch (newValueType)
	{
	case MachineValue::Type_t::boolean:
	{
		if (this->comboBox == nullptr)
		{
			this->comboBox = new DiscreetComboBox();
			this->comboBox->insertItem(static_cast<int>(indexType::falseValue), tr("False"));
			this->comboBox->insertItem(static_cast<int>(indexType::trueValue),  tr("True"));

			if (this->ignoreWheelEvents == true)
			{
				this->comboBox->setIgnoreWheelEvents(true);
			}

			this->layout()->addWidget(this->comboBox);
		}
		else
		{
			disconnect(this->comboBox, &DiscreetComboBox::currentIndexChanged, this, &ValueEditor::comboBoxIndexChangedEventHandler);
		}

		if (value.getBooleanValue() == false)
		{
			this->comboBox->setCurrentIndex(static_cast<int>(indexType::falseValue));
		}
		else
		{
			this->comboBox->setCurrentIndex(static_cast<int>(indexType::trueValue));
		}

		connect(this->comboBox, &DiscreetComboBox::currentIndexChanged, this, &ValueEditor::comboBoxIndexChangedEventHandler);
		break;
	}
	case MachineValue::Type_t::bitVector:
	{
		if (this->lineEdit == nullptr)
		{
			this->lineEdit = new ColoredLineEditor();

			this->layout()->addWidget(this->lineEdit);
		}
		else
		{
			disconnect(this->lineEdit, &ColoredLineEditor::editingFinished, this, &ValueEditor::textEditChangedEventHandler);
		}

		this->lineEdit->setText(value.toDisplayString());

		connect(this->lineEdit, &ColoredLineEditor::editingFinished, this, &ValueEditor::textEditChangedEventHandler);
		break;
	}
	case MachineValue::Type_t::nullType:
		// Checked before, should not happen
		break;
	}
}

MachineValue ValueEditor::getMachineValue() const
{
	if (this->valueType == MachineValue::Type_t::nullType) return MachineValue{};


	switch (this->valueType)
	{
	case MachineValue::Type_t::boolean:
		if (this->comboBox->currentIndex() == static_cast<int>(indexType::falseValue))
		{
			return BooleanValue::falseValue();
		}
		else
		{
			return BooleanValue::trueValue();
		}
		break;
	case MachineValue::Type_t::bitVector:
	{
		auto currentValue = BitVectorValue::fromRawString(this->lineEdit->text());

		if (this->bitVectorSize != 0)
		{
			if (currentValue.getSize() != this->bitVectorSize)
			{
				currentValue.resize(this->bitVectorSize);
			}
		}

		return currentValue;
		break;
	}
	case MachineValue::Type_t::nullType:
		// Checked before, should not happen
		break;
	}
}

void ValueEditor::setBitVectorSize(uint size)
{
	if (this->valueType != MachineValue::Type_t::bitVector) return;


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
}

void ValueEditor::setFocusOnShow(bool autoFocusOnNextShow)
{
	this->autoFocusOnNextShow = autoFocusOnNextShow;
}

void ValueEditor::setIgnoreWheelEvents(bool ignoreWheelEvents)
{
	this->ignoreWheelEvents = ignoreWheelEvents;

	if (this->comboBox != nullptr)
	{
		this->comboBox->setIgnoreWheelEvents(ignoreWheelEvents);
	}
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

	if (this->lineEdit != nullptr)
	{
		this->lineEdit->setFocus();
		this->lineEdit->selectAll();
	}
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

void ValueEditor::comboBoxIndexChangedEventHandler(int)
{
	emit this->valueChangedEvent(this);
}

void ValueEditor::textEditChangedEventHandler()
{
	emit this->valueChangedEvent(this);
}
