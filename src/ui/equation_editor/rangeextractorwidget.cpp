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
#include "rangeextractorwidget.h"

// Qt classes
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

// StateS classes
#include "lineeditwithupdownbuttons.h"
#include "equation.h"
#include "operand.h"


RangeExtractorWidget::RangeExtractorWidget(shared_ptr<Equation> equation, QWidget* parent) :
    EditableEquation(parent)
{
	this->equation = equation;

	connect(equation.get(), &Equation::equationTextChangedEvent, this, &RangeExtractorWidget::update);

	this->reset();
}

bool RangeExtractorWidget::validEdit()
{
	if (this->editMode == true)
	{
		this->setEdited(false);
		return true;
	}
	else
		return false;
}

bool RangeExtractorWidget::cancelEdit()
{
	if (this->editMode == true)
	{
		this->setEdited(false);
		return true;
	}
	else
		return false;
}

void RangeExtractorWidget::setEdited(bool edited)
{
	if (this->editMode != edited)
	{
		this->editMode = edited;

		this->reset();
	}
}

void RangeExtractorWidget::mousePressEvent(QMouseEvent* event)
{
	bool transmitEvent = true;

	if ( ( event->button() == Qt::LeftButton) && (this->editMode == false) )
	{
		this->setEdited(true);
		this->inMouseEvent = true;
		transmitEvent = false;
	}
	else if ( event->button() == Qt::RightButton)
	{
		this->inMouseEvent = true;
		transmitEvent = false;
	}

	if (transmitEvent)
		QWidget::mousePressEvent(event);
}

void RangeExtractorWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (!this->inMouseEvent)
		QWidget::mouseDoubleClickEvent(event);
}

void RangeExtractorWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (!this->inMouseEvent)
		QWidget::mouseDoubleClickEvent(event);
}

void RangeExtractorWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (!this->inMouseEvent)
	{
		QWidget::mouseDoubleClickEvent(event);
	}
	else
	{
		this->inMouseEvent = false;
	}
}

void RangeExtractorWidget::wheelEvent(QWheelEvent* event)
{
	shared_ptr<Equation> l_equation = this->equation.lock();

	if (l_equation != nullptr)
	{
		if (this->editMode == false)
		{
			QPoint eventPosition = QPoint(event->position().x(), event->position().y());

			QWidget* childUnderMouse = this->childAt(eventPosition);

			if (childUnderMouse == this->rangeLText)
			{
				if (event->angleDelta().y() > 0)
				{
					int newValue = this->rangeLText->text().toInt() + 1;

					auto operand = l_equation->getOperand(0);
					if (operand != nullptr)
					{
						if (newValue < (int)operand->getInitialValue().getSize())
						{
							emit rangeLChanged(newValue);
						}
					}
				}
				else
				{
					int newValue = this->rangeLText->text().toInt() - 1;

					if (l_equation->getRangeR() != -1)
					{
						if (newValue > l_equation->getRangeR())
						{
							emit rangeLChanged(newValue);
						}
					}
					else if (newValue >= 0)
					{
						emit rangeLChanged(newValue);
					}

				}
			}
			else if ( (this->rangeRText != nullptr) && (childUnderMouse == this->rangeRText) )
			{
				if (event->angleDelta().y() > 0)
				{
					int newValue = this->rangeRText->text().toInt() + 1;

					// Param 1 must be set first
					if (l_equation->getRangeL() != -1)
					{
						if (newValue < l_equation->getRangeL())
						{
							emit rangeRChanged(newValue);
						}
					}
				}
				else
				{
					int newValue = this->rangeRText->text().toInt() - 1;

					if (newValue >= 0)
					{
						emit rangeRChanged(newValue);
					}
				}
			}

		}
	}
}

void RangeExtractorWidget::update()
{
	shared_ptr<Equation> l_equation = this->equation.lock();
	QString text;

	if (l_equation != nullptr)
	{
		if (this->editMode == true)
		{
			// If we changed mode, we need to reset display
			if ( ( (this->rangeREditor != nullptr) && (l_equation->getRangeR() == -1) ) ||
			     ( (this->rangeREditor == nullptr) && (l_equation->getRangeR() != -1) ) )
			{
				this->reset();
			}
			else
			{
				if (l_equation->getRangeL() != -1)
				{
					text = QString::number(l_equation->getRangeL());
				}

				auto operand = l_equation->getOperand(0);
				if (operand != nullptr)
				{
					int variableSize = operand->getInitialValue().getSize();

					if (l_equation->getRangeR() != -1)
					{
						this->rangeLEditor->updateContent(l_equation->getRangeR(), variableSize-1, text);;

						text = QString::number(l_equation->getRangeR());
						this->rangeREditor->updateContent(0, l_equation->getRangeL(), text);
					}
					else
					{
						this->rangeLEditor->updateContent(0, variableSize-1, text);
					}
				}
			}
		}
		else
		{
			// If we changed mode, we need to reset display
			if ( ( (this->rangeRText != nullptr) && (l_equation->getRangeR() == -1) ) ||
			     ( (this->rangeRText == nullptr) && (l_equation->getRangeR() != -1) ) )
			{
				this->reset();
			}
			else
			{
				if (l_equation->getRangeL() != -1)
					text = QString::number(l_equation->getRangeL());
				else
					text = "…";

				this->rangeLText->setText(text);

				if (l_equation->getRangeR() != -1)
				{
					text = QString::number(l_equation->getRangeR());
					this->rangeRText->setText(text);
				}
			}
		}
	}
}

void RangeExtractorWidget::reset()
{
	qDeleteAll(this->children());
	delete this->layout();

	this->rangeLText   = nullptr;
	this->rangeRText   = nullptr;
	this->rangeLEditor = nullptr;
	this->rangeREditor = nullptr;

	shared_ptr<Equation> l_equation = this->equation.lock();
	if (l_equation == nullptr) return;


	QHBoxLayout* layout = new QHBoxLayout(this);
	QString text;

	QLabel* label = new QLabel("[");
	layout->addWidget(label);

	if (this->editMode == true)
	{
		if (l_equation->getRangeL() != -1)
			text = QString::number(l_equation->getRangeL());

		auto operand = l_equation->getOperand(0);
		if (operand != nullptr)
		{
			int variableSize = operand->getInitialValue().getSize();

			if (l_equation->getRangeR() != -1)
			{
				this->rangeLEditor = new LineEditWithUpDownButtons(l_equation->getRangeR()+1, variableSize-1, text);
				layout->addWidget(this->rangeLEditor);

				label = new QLabel("..");
				layout->addWidget(label);

				text = QString::number(l_equation->getRangeR());
				this->rangeREditor = new LineEditWithUpDownButtons(0, l_equation->getRangeL()-1, text);
				layout->addWidget(this->rangeREditor);

				connect(this->rangeREditor, &LineEditWithUpDownButtons::valueChanged, this, &RangeExtractorWidget::rangeRChanged);
			}
			else
			{
				this->rangeLEditor = new LineEditWithUpDownButtons(0, variableSize-1, text);
				layout->addWidget(this->rangeLEditor);
			}

			connect(this->rangeLEditor, &LineEditWithUpDownButtons::valueChanged, this, &RangeExtractorWidget::rangeLChanged);
		}
	}
	else
	{
		if (l_equation->getRangeL() != -1)
			text = QString::number(l_equation->getRangeL());
		else
			text = "…";

		this->rangeLText = new QLabel(text);
		this->rangeLText->setStyleSheet("QLabel {border: 2px solid green; border-radius: 5px}");

		layout->addWidget(this->rangeLText);

		if (l_equation->getRangeR() != -1)
		{
			label = new QLabel("..");
			layout->addWidget(label);

			text = QString::number(l_equation->getRangeR());

			this->rangeRText = new QLabel(text);
			this->rangeRText->setStyleSheet("QLabel {border: 2px solid green; border-radius: 5px}");

			layout->addWidget(this->rangeRText);
		}

	}

	label = new QLabel("]");
	layout->addWidget(label);
}

