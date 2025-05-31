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
#include "rangeeditor.h"

// Qt classes
#include <QHBoxLayout>
#include <QLabel>
#include <QInputEvent>

// StateS classes
#include "linewithupdownbuttonseditor.h"
#include "equation.h"
#include "operand.h"


RangeEditor::RangeEditor(shared_ptr<Equation> equation, QWidget* parent) :
	QWidget(parent)
{
	if (equation->getOperatorType() != OperatorType_t::extractOp) return;


	this->equation = equation;

	this->build();
}

/**
 * @brief RangeEditor::setMode changes the mode of the editor.
 * @param newMode New mode to apply to the editor
 * @param saveChanges Save current changes before swithing mode
 *        (only relevant if current mode is editor mode).
 * @return true if the mode was changed, false if there was no
 *         mode change to operate, or if the change failed.
 */
bool RangeEditor::setMode(Mode_t newMode, bool saveChanges)
{
	if (this->equation == nullptr) return false;

	if (this->mode == newMode) return false;


	if (newMode == Mode_t::editor_mode)
	{
		// The operand CAN be null:
		// do not allow triggering editor mode in that case.
		if (this->equation->getOperand(0) != nullptr)
		{
			emit this->beginEditEvent();

			this->mode = Mode_t::editor_mode;
		}
	}
	else  // (newMode == Mode_t::compact_mode)
	{
		if (saveChanges == true)
		{
			int rangeL = 0;
			int rangeR = -1;

			if (this->rangeLEditor != nullptr) // This should be true, but check anyway
			{
				rangeL = this->rangeLEditor->getValue();
			}

			if (this->rangeREditor != nullptr)
			{
				rangeR = this->rangeREditor->getValue();
			}

			// Make sure range values are acceptable
			bool valueAccepted = true;

			auto operand = this->equation->getOperand(0);
			// If operand is null, just accept any value
			// as equation is invalid any way.
			if (operand != nullptr)
			{
				auto value = operand->getInitialValue();
				int operandSize = value.getSize();

				if (rangeL <= rangeR)      valueAccepted = false;
				if (rangeR < -1)           valueAccepted = false;
				if (rangeL >= operandSize) valueAccepted = false;
			}

			if (valueAccepted == true)
			{
				this->equation->setRange(rangeL, rangeR);
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

	return true;
}

void RangeEditor::setExtractSingleBit()
{
	int rangeL = this->equation->getRangeL();
	this->equation->setRange(rangeL, -1);

	this->clear();
	this->build();
}

void RangeEditor::setExtractRange()
{
	int rangeL = this->equation->getRangeL();
	int rangeR;

	if (rangeL > 0)
	{
		rangeR = rangeL-1;
	}
	else
	{
		rangeR = 0;
		rangeL = 1;
	}

	this->equation->setRange(rangeL, rangeR);

	this->clear();
	this->build();
}

void RangeEditor::refresh()
{
	this->clear();
	this->setMode(RangeEditor::Mode_t::compact_mode, false);
	this->build();
}

void RangeEditor::mousePressEvent(QMouseEvent* event)
{
	bool eventAccepted = false;

	if ( (event->button() == Qt::LeftButton) && (this->mode == Mode_t::compact_mode) )
	{
		this->setMode(Mode_t::editor_mode);
		eventAccepted = true;
	}

	if (eventAccepted == false)
	{
		event->ignore();
	}
}

void RangeEditor::wheelEvent(QWheelEvent* event)
{
	if (this->equation == nullptr) return;

	if (this->mode != Mode_t::compact_mode) return;

	if (this->rangeLText == nullptr) return;

	auto operand = this->equation->getOperand(0);
	if (operand == nullptr) return;


	auto value = operand->getInitialValue();
	int vectorSize = value.getSize();

	int upperLBound = vectorSize-1;
	int lowerRBound = 0;

	int currentRangeL = this->equation->getRangeL();
	int upperRBound = currentRangeL - 1;

	int currentRangeR = this->equation->getRangeR();
	int lowerLBound = currentRangeR + 1;

	auto eventPosition = QPoint(event->position().x(), event->position().y());
	auto childUnderMouse = this->childAt(eventPosition);

	if (childUnderMouse == this->rangeLText)
	{
		if (currentRangeL != -1)
		{
			int newValue;

			if (event->angleDelta().y() > 0)
			{
				newValue = currentRangeL + 1;
			}
			else
			{
				newValue = currentRangeL - 1;
			}

			if ( (newValue <= upperLBound) && (newValue >= lowerLBound) )
			{
				this->setRangeL(newValue);
			}
		}
		else
		{
			this->setRangeL(vectorSize-1);
		}
	}
	else if ( (this->rangeRText != nullptr) && (childUnderMouse == this->rangeRText) )
	{
		if (currentRangeR != -1)
		{
			int newValue;

			if (event->angleDelta().y() > 0)
			{
				newValue = currentRangeR + 1;
			}
			else
			{
				newValue = currentRangeR - 1;
			}

			if ( (newValue <= upperRBound) && (newValue >= lowerRBound) )
			{
				this->setRangeR(newValue);
			}
		}
		else
		{
			this->setRangeR(0);
		}
	}
}

void RangeEditor::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key::Key_Return)
	{
		if (this->mode == Mode_t::editor_mode)
		{
			this->setMode(RangeEditor::Mode_t::compact_mode, true);
			transmitEvent = false;
		}
	}
	else if (event->key() == Qt::Key::Key_Escape)
	{
		if (this->mode == Mode_t::editor_mode)
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

void RangeEditor::setRangeL(uint newValue)
{
	int rangeR = this->equation->getRangeR();
	this->equation->setRange(newValue, rangeR);

	if (this->mode == Mode_t::compact_mode)
	{
		if (this->rangeLText == nullptr) return;


		this->rangeLText->setText(QString::number(newValue));
	}
}

void RangeEditor::setRangeR(uint newValue)
{
	int rangeL = this->equation->getRangeL();
	this->equation->setRange(rangeL, newValue);

	if (this->mode == Mode_t::compact_mode)
	{
		if (this->rangeRText == nullptr) return;


		this->rangeRText->setText(QString::number(newValue));
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
	if (this->equation == nullptr) return;


	auto mainLayout = new QHBoxLayout(this);
	mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

	mainLayout->addStretch();

	auto label = new QLabel("[");
	label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	mainLayout->addWidget(label);

	int currentRangeL = this->equation->getRangeL();
	int currentRangeR = this->equation->getRangeR();

	QString text;
	if (this->mode == Mode_t::editor_mode)
	{
		mainLayout->setSpacing(0);

		if (currentRangeL != -1)
		{
			text = QString::number(currentRangeL);
		}

		auto operand = this->equation->getOperand(0);
		if (operand != nullptr)
		{
			auto value = operand->getInitialValue();
			int variableSize = value.getSize();

			if (currentRangeR != -1)
			{
				this->rangeLEditor = new LineWithUpDownButtonsEditor(currentRangeR+1, variableSize-1, text);
				connect(this->rangeLEditor, &LineWithUpDownButtonsEditor::valueChanged, this, &RangeEditor::lEditorValueChangedEventHandler);
				mainLayout->addWidget(this->rangeLEditor);

				label = new QLabel("..");
				mainLayout->addWidget(label);

				text = QString::number(currentRangeR);
				this->rangeREditor = new LineWithUpDownButtonsEditor(0, currentRangeL-1, text);
				connect(this->rangeREditor, &LineWithUpDownButtonsEditor::valueChanged, this, &RangeEditor::rEditorValueChangedEventHandler);
				mainLayout->addWidget(this->rangeREditor);
			}
			else
			{
				this->rangeLEditor = new LineWithUpDownButtonsEditor(0, variableSize-1, text);
				mainLayout->addWidget(this->rangeLEditor);
			}
		}
	}
	else // (this->getEditMode() == EditionMode_t::display_mode)
	{
		this->rangeLText = new QLabel();
		if (currentRangeL != -1)
		{
			text = QString::number(currentRangeL);
			this->rangeLText->setStyleSheet("QLabel {border: 1px solid green; border-radius: 5px}");
		}
		else
		{
			text = "…";
			this->rangeLText->setStyleSheet("QLabel {border: 1px solid red; border-radius: 5px}");
		}
		this->rangeLText->setText(text);

		mainLayout->addWidget(this->rangeLText);

		if (currentRangeR != -1)
		{
			label = new QLabel("..");
			mainLayout->addWidget(label);

			this->rangeRText = new QLabel();
			if (currentRangeR != -1)
			{
				text = QString::number(currentRangeR);
				this->rangeRText->setStyleSheet("QLabel {border: 1px solid green; border-radius: 5px}");
			}
			else
			{
				text = "…";
				this->rangeRText->setStyleSheet("QLabel {border: 1px solid red; border-radius: 5px}");
			}
			this->rangeRText->setText(text);


			mainLayout->addWidget(this->rangeRText);
		}
	}

	label = new QLabel("]");
	label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	mainLayout->addWidget(label);
	mainLayout->addStretch();
}
