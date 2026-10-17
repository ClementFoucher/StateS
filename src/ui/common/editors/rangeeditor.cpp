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
#include "rangeeditor.h"

// Qt
#include <QHBoxLayout>
#include <QLabel>
#include <QInputEvent>

// StateS
#include "linewithupdownbuttonseditor.h"
#include "equation.h"
#include "operand.h"


RangeEditor::RangeEditor(std::shared_ptr<Equation> equation, QWidget* parent) :
	QWidget(parent)
{
	if (equation->getOperator() != Equation::Operator_t::extractOp) return;

	auto operand0 = equation->getOperand(0);
	if (operand0 == nullptr) return;
	if (operand0->getType() != MachineValue::Type_t::bitVector) return;

	auto operand1 = equation->getOperand(1);
	if (operand1 == nullptr) return;
	if (operand1->getType() != MachineValue::Type_t::natural) return;

	if (equation->getOperandCount() == 3)
	{
		auto operand2 = equation->getOperand(2);
		if (operand2 == nullptr) return;
		if (operand2->getType() != MachineValue::Type_t::natural) return;
	}

	auto value = operand0->getInitialValue().getBitVectorValue();
	this->vectorSize = value.getSize();
	if (this->vectorSize == 0) return;


	this->equation = equation;
	this->isValid = true;

	this->build();
}

void RangeEditor::setExtractSingleBit()
{
	if (this->isValid == false) return;

	if (this->equation->getOperandCount() != 3) return;


	this->equation->decreaseOperandCount();

	this->clear();
	this->build();
}

void RangeEditor::setExtractRange()
{
	if (this->isValid == false) return;

	if (this->vectorSize < 2) return;

	if (this->equation->getOperandCount() != 2) return;


	this->equation->increaseOperandCount();

	unsigned int rangeL = this->equation->getOperand(1)->getInitialValue().getNaturalValue();
	if (rangeL > 0)
	{
		this->equation->setOperand(2, NaturalValue{rangeL - 1});
	}
	else
	{
		this->equation->setOperand(1, NaturalValue{1});
		this->equation->setOperand(2, NaturalValue{0});
	}

	this->clear();
	this->build();
}

void RangeEditor::mousePressEvent(QMouseEvent* event)
{
	if ( (event->button() == Qt::LeftButton) && (this->mode == Mode_t::compact_mode) )
	{
		this->setMode(Mode_t::editor_mode, false);
	}
	else
	{
		QWidget::mousePressEvent(event);
	}
}

void RangeEditor::wheelEvent(QWheelEvent* event)
{
	if (this->isValid == false)
	{
		QWidget::wheelEvent(event);
		return;
	}

	if (this->mode != Mode_t::compact_mode)
	{
		QWidget::wheelEvent(event);
		return;
	}


	uint currentOperandRank;
	uint upperBound;
	uint lowerBound;
	if ( (this->rangeLText != nullptr) && (this->rangeLText->underMouse() == true) )
	{
		// Left range or bit to extract
		currentOperandRank = 1;

		upperBound = this->vectorSize-1;
		if (this->equation->getOperandCount() == 3)
		{
			lowerBound = this->equation->getOperand(2)->getInitialValue().getNaturalValue() + 1;
		}
		else
		{
			lowerBound = 0;
		}
	}
	else if ( (this->rangeRText != nullptr) && (this->rangeRText->underMouse() == true) )
	{
		// Right range
		currentOperandRank = 2;

		upperBound = this->equation->getOperand(1)->getInitialValue().getNaturalValue() - 1;
		lowerBound = 0;
	}
	else
	{
		QWidget::wheelEvent(event);
		return;
	}

	uint currentValue = this->equation->getOperand(currentOperandRank)->getInitialValue().getNaturalValue();
	uint newValue;
	if (event->angleDelta().y() > 0)
	{
		newValue = currentValue + 1;
	}
	else // (event->angleDelta().y() < 0)
	{
		newValue = currentValue - 1;
	}

	if ( (newValue <= upperBound) && (newValue >= lowerBound) )
	{
		if (currentOperandRank == 1)
		{
			this->setRangeL(newValue);
		}
		else // (currentRangeRank == 2)
		{
			this->setRangeR(newValue);
		}
	}
}

void RangeEditor::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (this->mode == Mode_t::editor_mode)
	{
		if (event->key() == Qt::Key::Key_Return)
		{
			this->setMode(RangeEditor::Mode_t::compact_mode, true);
			transmitEvent = false;
		}
		else if (event->key() == Qt::Key::Key_Escape)
		{
			this->setMode(RangeEditor::Mode_t::compact_mode, false);
			transmitEvent = false;
		}
	}

	if (transmitEvent == true)
	{
		QWidget::keyPressEvent(event);
	}
}

void RangeEditor::lEditorValueChangedEventHandler(int newValue)
{
	if (this->rangeREditor == nullptr) return;


	this->rangeREditor->setMaxValue(newValue-1);
}

void RangeEditor::rEditorValueChangedEventHandler(int newValue)
{
	if (this->rangeLEditor == nullptr) return;


	this->rangeLEditor->setMinValue(newValue+1);
}

/**
 * @brief RangeEditor::setMode changes the mode of the editor.
 * @param newMode New mode to apply to the editor
 * @param saveChanges Save current changes before swithing mode
 *        (only relevant if current mode is editor mode).
 */
void RangeEditor::setMode(Mode_t newMode, bool saveChanges)
{
	if (this->isValid == false) return;

	if (this->mode == newMode) return;


	if (newMode == Mode_t::editor_mode)
	{
		this->mode = Mode_t::editor_mode;
	}
	else // (newMode == Mode_t::compact_mode)
	{
		if (saveChanges == true)
		{
			auto rangeL = NaturalValue{static_cast<unsigned int>(this->rangeLEditor->getValue())};
			this->equation->setOperand(1, rangeL);

			if (this->rangeREditor != nullptr)
			{
				auto rangeR = NaturalValue{static_cast<unsigned int>(this->rangeREditor->getValue())};
				this->equation->setOperand(2, rangeR);
			}
		}

		this->mode = Mode_t::compact_mode;
	}

	this->clear();
	this->build();

	if (this->mode == Mode_t::editor_mode)
	{
		if (this->rangeLEditor != nullptr)
		{
			this->rangeLEditor->setFocus();
		}
	}
}

void RangeEditor::setRangeL(uint newValue)
{
	if (this->isValid == false) return;


	this->equation->setOperand(1, NaturalValue{newValue});

	if (this->mode == Mode_t::compact_mode)
	{
		if (this->rangeLText == nullptr) return;


		this->rangeLText->setText(QString::number(newValue));
	}
}

void RangeEditor::setRangeR(uint newValue)
{
	if (this->isValid == false) return;


	this->equation->setOperand(2, NaturalValue{newValue});

	if (this->mode == Mode_t::compact_mode)
	{
		if (this->rangeRText == nullptr) return;


		this->rangeRText->setText(QString::number(newValue));
	}
}

void RangeEditor::clear()
{
	qDeleteAll(this->children());
	delete this->layout();

	this->rangeLText   = nullptr;
	this->rangeRText   = nullptr;
	this->rangeLEditor = nullptr;
	this->rangeREditor = nullptr;
}

void RangeEditor::build()
{
	if (this->isValid == false) return;


	auto mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(0);
	mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

	auto label = new QLabel("[");
	mainLayout->addWidget(label);

	uint currentRangeL = this->equation->getOperand(1)->getInitialValue().getNaturalValue();
	if (this->mode == Mode_t::editor_mode)
	{

		uint lowerBound;
		if (this->equation->getOperandCount() == 3)
		{
			lowerBound = this->equation->getOperand(2)->getInitialValue().getNaturalValue() + 1;
		}
		else
		{
			lowerBound = 0;
		}

		this->rangeLEditor = new LineWithUpDownButtonsEditor(lowerBound, this->vectorSize-1, QString::number(currentRangeL));
		mainLayout->addWidget(this->rangeLEditor);
	}
	else // (this->mode == Mode_t::compact_mode)
	{
		this->rangeLText = new QLabel(QString::number(currentRangeL));
		this->rangeLText->setStyleSheet("QLabel {border: 1px solid green; border-radius: 5px}");

		mainLayout->addWidget(this->rangeLText);
	}

	if (this->equation->getOperandCount() == 3)
	{
		label = new QLabel("..");
		mainLayout->addWidget(label);

		uint currentRangeR = this->equation->getOperand(2)->getInitialValue().getNaturalValue();
		if (this->mode == Mode_t::editor_mode)
		{
			this->rangeREditor = new LineWithUpDownButtonsEditor(0, currentRangeL-1, QString::number(currentRangeR));
			mainLayout->addWidget(this->rangeREditor);

			connect(this->rangeLEditor, &LineWithUpDownButtonsEditor::valueChanged, this, &RangeEditor::lEditorValueChangedEventHandler);
			connect(this->rangeREditor, &LineWithUpDownButtonsEditor::valueChanged, this, &RangeEditor::rEditorValueChangedEventHandler);
		}
		else // (this->mode == Mode_t::compact_mode)
		{
			this->rangeRText = new QLabel(QString::number(currentRangeR));
			this->rangeRText->setStyleSheet("QLabel {border: 1px solid green; border-radius: 5px}");

			mainLayout->addWidget(this->rangeRText);
		}
	}

	label = new QLabel("]");
	mainLayout->addWidget(label);
}
