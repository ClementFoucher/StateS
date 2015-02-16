/*
 * Copyright © 2014 Clément Foucher
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
#include "langselectiondialog.h"

// Qt classes
#include <QVBoxLayout>

// StateS classes
#include "reactivebutton.h"
#include "states.h"


LangSelectionDialog::LangSelectionDialog(QApplication* application, QWidget* parent) :
    QDialog(parent)
{
    this->application = application;
    this->setWindowIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/StateS"))));

    this->translator = new QTranslator();
    this->translator->load(":/translations/français/français");

    QVBoxLayout* layout = new QVBoxLayout(this);

    this->title = new QLabel();
    this->title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    this->buttonEnglish = new ReactiveButton("English");
    connect(this->buttonEnglish, &QAbstractButton::clicked, this, &LangSelectionDialog::accept);
    layout->addWidget(this->buttonEnglish);

    this->buttonFrench = new ReactiveButton("Français");
    connect(this->buttonFrench, &ReactiveButton::mouseEnterEvent, this, &LangSelectionDialog::setFrench);
    connect(this->buttonFrench, &ReactiveButton::mouseLeaveEvent, this, &LangSelectionDialog::setEnglish);
    connect(this->buttonFrench, &QAbstractButton::clicked,        this, &LangSelectionDialog::accept);
    layout->addWidget(this->buttonFrench);

    retranslateUi();
}

void LangSelectionDialog::retranslateUi()
{
    this->title->setText("<b>" + tr("Choose your language") + "</b>");
}

void LangSelectionDialog::setEnglish()
{
    this->application->removeTranslator(translator);
    retranslateUi();
}

void LangSelectionDialog::setFrench()
{
    this->application->installTranslator(translator);
    retranslateUi();
}

QTranslator* LangSelectionDialog::getTranslator()
{
    return this->translator;
}
