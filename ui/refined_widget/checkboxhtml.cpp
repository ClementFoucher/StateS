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
#include "checkboxhtml.h"

// Qt classes
#include <QCheckBox>
#include <QHBoxLayout>
#include <QCoreApplication>
#include <QMouseEvent>

// StateS classes
#include "labelwithclickevent.h"


CheckBoxHtml::CheckBoxHtml(const QString& text, Qt::AlignmentFlag boxAlign, bool allowLink, QWidget* parent) :
    QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    this->checkBox = new QCheckBox();
    connect(this->checkBox, &QCheckBox::toggled, this, &CheckBoxHtml::toggled);

    this->label = new LabelWithClickEvent(text);
    connect(this->label, &LabelWithClickEvent::clicked, this, &CheckBoxHtml::event);

    this->label->setTextFormat(Qt::RichText);
    if (allowLink)
    {
        this->label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        this->label->setOpenExternalLinks(true);
    }

    if (boxAlign == Qt::AlignmentFlag::AlignLeft)
    {
        layout->addWidget(this->checkBox);
        layout->addWidget(this->label);
    }
    else
    {
        layout->addWidget(this->label);
        layout->addWidget(this->checkBox);
    }
}

bool CheckBoxHtml::event(QEvent* e)
{
    static bool doNotResend = false; // Used as we transmit event to a children: do not retransmit in a loop if it gives it back to its parent (this object)

    if (doNotResend == false)
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(e);

        if ( (mouseEvent != nullptr) && (mouseEvent->button() == Qt::MouseButton::LeftButton) )
        {
            if ( (mouseEvent->type() == QEvent::Type::MouseButtonPress) ||
                 (mouseEvent->type() == QEvent::Type::MouseButtonRelease) ||
                 (mouseEvent->type() == QEvent::Type::MouseButtonDblClick) ||
                 (mouseEvent->type() == QEvent::Type::MouseMove)
                 )
            {

                // Fake an event on the checkbox so that it handles it
                QMouseEvent newEvent(mouseEvent->type(),
                                     QPointF(0,0),
                                     mouseEvent->button(),
                                     mouseEvent->buttons(),
                                     mouseEvent->modifiers()
                                     );

                doNotResend = true;
                bool res = QCoreApplication::sendEvent(this->checkBox, &newEvent);
                doNotResend = false;
                return res;
            }
        }
    }

    return QWidget::event(e);
}

void CheckBoxHtml::setText(QString newText)
{
    this->label->setText(newText);
}

void CheckBoxHtml::setChecked(bool check)
{
    this->checkBox->setChecked(check);
}

bool CheckBoxHtml::isChecked()
{
    return this->checkBox->isChecked();
}

