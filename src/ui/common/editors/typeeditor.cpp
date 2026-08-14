/*
 * Copyright © 2026 Clément Foucher
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
#include "typeeditor.h"

// Qt
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QFocusEvent>

// StateS
#include "selfmanageddynamiclineeditor.h"


TypeEditor::TypeEditor(QWidget* parent) :
	QWidget(parent)
{
	//
	// Build subwidget
	this->typeComboBox = new QComboBox();
	this->typeComboBox->insertItem(static_cast<int>(indexType::boolean),   tr("Boolean"));
	this->typeComboBox->insertItem(static_cast<int>(indexType::bitVector), tr("Bit vector"));

	//
	// Build complete rendering
	auto editorLayout = new QHBoxLayout(this);
	editorLayout->setContentsMargins(0, 0, 0, 0);

	editorLayout->addWidget(this->typeComboBox);

	//
	// Connect signals
	connect(this->typeComboBox, &QComboBox::currentIndexChanged, this, &TypeEditor::selectedTypeChangedEventHandler);
}

TypeEditor::~TypeEditor()
{
	// Because they replace() each other, one of these
	// may not be a children of this: delete manually
	delete(this->sizeLineEdit);
	delete(this->sizeLabel);

	this->sizeLineEdit = nullptr;
	this->sizeLabel    = nullptr;
}

void TypeEditor::setCurrentType(MachineValue::Type_t type)
{
	if (type == MachineValue::Type_t::nullType) return;


	if (type != MachineValue::Type_t::bitVector)
	{
		delete this->sizeLineEdit;
		delete this->sizeLabel;
		delete this->openingBracket;
		delete this->closingBracket;

		this->sizeLineEdit   = nullptr;
		this->sizeLabel      = nullptr;
		this->openingBracket = nullptr;
		this->closingBracket = nullptr;
	}

	disconnect(this->typeComboBox, &QComboBox::currentIndexChanged, this, &TypeEditor::selectedTypeChangedEventHandler);
	switch (type)
	{
	case MachineValue::Type_t::boolean:
		this->typeComboBox->setCurrentIndex(static_cast<int>(indexType::boolean));
		break;
	case MachineValue::Type_t::bitVector:
	{
		this->typeComboBox->setCurrentIndex(static_cast<int>(indexType::bitVector));

		this->sizeLabel = new QLabel(QString::number(this->bitVectorSize));

		this->openingBracket = new QLabel("[");
		this->closingBracket = new QLabel("]");

		this->layout()->addWidget(this->openingBracket);
		this->layout()->addWidget(this->sizeLabel);
		this->layout()->addWidget(this->closingBracket);

		break;
	}
	case MachineValue::Type_t::nullType:
		// Checked previously: should not happen
		break;
	}
	connect(this->typeComboBox, &QComboBox::currentIndexChanged, this, &TypeEditor::selectedTypeChangedEventHandler);
}

MachineValue::Type_t TypeEditor::getCurrentType() const
{
	auto comboBoxIndex = this->typeComboBox->currentIndex();
	if (comboBoxIndex == static_cast<int>(indexType::boolean))
	{
		return MachineValue::Type_t::boolean;
	}
	else if (comboBoxIndex == static_cast<int>(indexType::bitVector))
	{
		return MachineValue::Type_t::bitVector;
	}

	return MachineValue::Type_t::nullType;
}

void TypeEditor::setBitVectorSize(uint size)
{
	if (size == 0) return;


	this->bitVectorSize = size;

	if (this->sizeLineEdit != nullptr)
	{
		this->sizeLineEdit->setText(QString::number(size));
	}

	if (this->sizeLabel != nullptr)
	{
		this->sizeLabel->setText(QString::number(size));
	}
}

uint TypeEditor::getBitVectorSize() const
{
	if (this->sizeLineEdit != nullptr)
	{
		return this->sizeLineEdit->text().toUInt();
	}
	else
	{
		return this->bitVectorSize;
	}
}

void TypeEditor::triggerEditBitVectorSize()
{
	if (this->sizeLabel == nullptr) return;


	if (this->sizeLineEdit == nullptr)
	{
		this->sizeLineEdit = new SelfManagedDynamicLineEditor();
		this->sizeLineEdit->setValidator(new QIntValidator(0, BitVectorValue::maxSize()));
		this->sizeLineEdit->setMaximumSize(25, this->sizeLineEdit->maximumSize().height());
	}
	else
	{
		this->sizeLineEdit->show();
	}

	this->layout()->replaceWidget(this->sizeLabel, this->sizeLineEdit);
	this->sizeLabel->hide();

	this->sizeLineEdit->setText(QString::number(this->bitVectorSize));

	connect(this->sizeLineEdit, &SelfManagedDynamicLineEditor::newTextAvailableEvent, this, &TypeEditor::sizeChangedEventHandler);
	connect(this->sizeLineEdit, &SelfManagedDynamicLineEditor::userCancelEvent,       this, &TypeEditor::sizeEditCanceledEventHandler);
	this->sizeLineEdit->installEventFilter(this);

	this->sizeLineEdit->setFocus();
	this->sizeLineEdit->selectAll();
}

void TypeEditor::setErroneous(bool erroneous)
{
	if (this->sizeLineEdit == nullptr) return;


	this->sizeLineEdit->setErroneous(erroneous);
}

bool TypeEditor::getIsErroneous() const
{
	if (this->sizeLineEdit == nullptr) return false;


	return this->sizeLineEdit->getIsErroneous();
}

void TypeEditor::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		this->triggerEditBitVectorSize();
	}

	QWidget::mousePressEvent(event);
}

bool TypeEditor::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == this->sizeLineEdit)
	{
		auto focusEvent = static_cast<QFocusEvent*>(event);

		if (focusEvent != nullptr)
		{
			if (focusEvent->lostFocus() == true)
			{
				this->sizeChangedEventHandler(QString());
				return true;
			}
		}
	}

	return QWidget::eventFilter(watched, event);
}

void TypeEditor::selectedTypeChangedEventHandler(int)
{
	emit this->typeChangedEvent(this);
	// Do not do anything after this line:
	// when a type change is triggered, the editor
	// is destroyed (close persistent editors)
	// then recreated (open persitent editors)
}

void TypeEditor::sizeChangedEventHandler(QString)
{
	if (this->sizeLineEdit == nullptr) return;


	emit this->typeChangedEvent(this);
	// Do not do anything after this line:
	// when a type change is triggered, the editor
	// is destroyed (close persistent editors)
	// then recreated (open persitent editors)
}

void TypeEditor::sizeEditCanceledEventHandler()
{
	if (this->sizeLineEdit == nullptr) return;

	if (this->sizeLabel == nullptr) return;


	// Do not delete the line edit, as we may be in an event loop
	// triggered by it. Simply disconnect its events and hide it.
	disconnect(this->sizeLineEdit, &SelfManagedDynamicLineEditor::newTextAvailableEvent, this, &TypeEditor::sizeChangedEventHandler);
	disconnect(this->sizeLineEdit, &SelfManagedDynamicLineEditor::userCancelEvent,       this, &TypeEditor::sizeEditCanceledEventHandler);
	this->sizeLineEdit->removeEventFilter(this);

	this->sizeLabel->setText(QString::number(this->bitVectorSize));
	this->sizeLabel->show();
	this->layout()->replaceWidget(this->sizeLineEdit, this->sizeLabel);
	this->sizeLineEdit->hide();
	this->sizeLineEdit->setErroneous(false);
}
