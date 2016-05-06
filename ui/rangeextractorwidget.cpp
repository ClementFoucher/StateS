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
#include "rangeextractorwidget.h"

// Qt classes
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

// StateS classes
#include "lineeditwithupdownbuttons.h"
#include "equation.h"


RangeExtractorWidget::RangeExtractorWidget(shared_ptr<Equation> equation, QWidget* parent) :
    EditableEquation(parent)
{
    this->equation = equation;

    connect(equation.get(), &Equation::signalStaticConfigurationChangedEvent, this, &RangeExtractorWidget::update);

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
            QWidget* childUnderMouse = this->childAt(event->pos());

            if (childUnderMouse == this->param1Text)
            {
                if (event->delta() > 0)
                {
                    int newValue = this->param1Text->text().toInt() + 1;

                    if (newValue < (int)l_equation->getOperand(0)->getSize()) // Throws StatesException - Extract op aways has operand 0 - ignored
                        emit value1Changed(newValue);
                }
                else
                {
                    int newValue = this->param1Text->text().toInt() - 1;

                    if (l_equation->getParam2() != -1)
                    {
                        if (newValue > l_equation->getParam2())
                            emit value1Changed(newValue);
                    }
                    else if (newValue >= 0)
                    {
                        emit value1Changed(newValue);
                    }

                }
            }
            else if ( (this->param2Text != nullptr) && (this->childAt(event->pos()) == this->param2Text) )
            {
                if (event->delta() > 0)
                {
                    int newValue = this->param2Text->text().toInt() + 1;

                    // Param 1 must be set first
                    if (l_equation->getParam1() != -1)
                    {
                        if (newValue < l_equation->getParam1())
                            emit value2Changed(newValue);
                    }
                }
                else
                {
                    int newValue = this->param2Text->text().toInt() - 1;

                    if (newValue >= 0)
                        emit value2Changed(newValue);
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
            if ( ( (this->param2Editor != nullptr) && (l_equation->getParam2() == -1) ) ||
                 ( (this->param2Editor == nullptr) && (l_equation->getParam2() != -1) ) )
            {
                this->reset();
            }
            else
            {
                if (l_equation->getParam1() != -1)
                    text = QString::number(l_equation->getParam1());

                int signalSize = l_equation->getOperand(0)->getSize(); // Throws StatesException - Extract op aways has operand 0 - ignored

                if (l_equation->getParam2() != -1)
                {
                    this->param1Editor->updateContent(l_equation->getParam2(), signalSize-1, text);;

                    text = QString::number(l_equation->getParam2());
                    this->param2Editor->updateContent(0, l_equation->getParam1(), text);
                }
                else
                {
                    this->param1Editor->updateContent(0, signalSize-1, text);
                }
            }
        }
        else
        {
            // If we changed mode, we need to reset display
            if ( ( (this->param2Text != nullptr) && (l_equation->getParam2() == -1) ) ||
                 ( (this->param2Text == nullptr) && (l_equation->getParam2() != -1) ) )
            {
                this->reset();
            }
            else
            {
                if (l_equation->getParam1() != -1)
                    text = QString::number(l_equation->getParam1());
                else
                    text = "…";

                this->param1Text->setText(text);

                if (l_equation->getParam2() != -1)
                {
                    text = QString::number(l_equation->getParam2());
                    this->param2Text->setText(text);
                }
            }
        }
    }
}

void RangeExtractorWidget::reset()
{
    qDeleteAll(this->children());
    delete this->layout();

    this->param1Text   = nullptr;
    this->param2Text   = nullptr;
    this->param1Editor = nullptr;
    this->param2Editor = nullptr;

    shared_ptr<Equation> l_equation = this->equation.lock();

    if (l_equation != nullptr)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        QString text;

        QLabel* label = new QLabel("[");
        layout->addWidget(label);

        if (this->editMode == true)
        {
            if (l_equation->getParam1() != -1)
                text = QString::number(l_equation->getParam1());

            int signalSize = l_equation->getOperand(0)->getSize(); // Throws StatesException - Extract op aways has operand 0 - ignored

            if (l_equation->getParam2() != -1)
            {
                this->param1Editor = new LineEditWithUpDownButtons(l_equation->getParam2()+1, signalSize-1, text);
                layout->addWidget(this->param1Editor);

                label = new QLabel("..");
                layout->addWidget(label);

                text = QString::number(l_equation->getParam2());
                this->param2Editor = new LineEditWithUpDownButtons(0, l_equation->getParam1()-1, text);
                layout->addWidget(this->param2Editor);

                connect(this->param2Editor, &LineEditWithUpDownButtons::valueChanged, this, &RangeExtractorWidget::value2Changed);
            }
            else
            {
                this->param1Editor = new LineEditWithUpDownButtons(0, signalSize-1, text);
                layout->addWidget(this->param1Editor);
            }

            connect(this->param1Editor, &LineEditWithUpDownButtons::valueChanged, this, &RangeExtractorWidget::value1Changed);
        }
        else
        {
            if (l_equation->getParam1() != -1)
                text = QString::number(l_equation->getParam1());
            else
                text = "…";

            this->param1Text = new QLabel(text);
            this->param1Text->setStyleSheet("QLabel {border: 2px solid green; border-radius: 5px}");

            layout->addWidget(this->param1Text);

            if (l_equation->getParam2() != -1)
            {
                label = new QLabel("..");
                layout->addWidget(label);

                text = QString::number(l_equation->getParam2());

                this->param2Text = new QLabel(text);
                this->param2Text->setStyleSheet("QLabel {border: 2px solid green; border-radius: 5px}");

                layout->addWidget(this->param2Text);
            }


        }

        label = new QLabel("]");
        layout->addWidget(label);

    }
}

