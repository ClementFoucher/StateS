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
#include "langselectiondialog.h"

// Qt classes
#include <QVBoxLayout>
#include <QApplication>
#include <QTranslator>
#include <QLabel>

// StateS classes
#include "reactivebutton.h"
#include "svgimagegenerator.h"


LangSelectionDialog::LangSelectionDialog(shared_ptr<QApplication> application, QWidget* parent) :
    QDialog(parent)
{
    this->application = application;
    this->setWindowIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/StateS"))));

    this->translator = shared_ptr<QTranslator>(new QTranslator());
    this->translator->load(":/translations/français/français");

    this->setWindowTitle("StateS");

    QVBoxLayout* layout = new QVBoxLayout(this);

    this->title = new QLabel();
    this->title->setAlignment(Qt::AlignCenter);
    layout->addWidget(this->title);

    ReactiveButton* buttonEnglish = new ReactiveButton("English");
    connect(buttonEnglish, &ReactiveButton::mouseEnterEvent,      this, &LangSelectionDialog::setEnglish);
    connect(buttonEnglish, &ReactiveButton::keyboardFocusInEvent, this, &LangSelectionDialog::setEnglish);
    connect(buttonEnglish, &QAbstractButton::clicked,             this, &LangSelectionDialog::accept);
    layout->addWidget(buttonEnglish);

    ReactiveButton* buttonFrench = new ReactiveButton("Français");
    connect(buttonFrench, &ReactiveButton::mouseEnterEvent,      this, &LangSelectionDialog::setFrench);
    connect(buttonFrench, &ReactiveButton::keyboardFocusInEvent, this, &LangSelectionDialog::setFrench);
    connect(buttonFrench, &QAbstractButton::clicked,             this, &LangSelectionDialog::accept);
    layout->addWidget(buttonFrench);

    retranslateUi();
}

void LangSelectionDialog::retranslateUi()
{
    this->title->setText("<b>" + tr("Choose your language") + "</b>");
}

void LangSelectionDialog::setEnglish()
{
    this->application->removeTranslator(translator.get());
    retranslateUi();
}

void LangSelectionDialog::setFrench()
{
    this->application->installTranslator(translator.get());
    retranslateUi();
}

shared_ptr<QTranslator> LangSelectionDialog::getTranslator()
{
    return this->translator;
}
