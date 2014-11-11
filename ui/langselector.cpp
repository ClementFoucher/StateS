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

#include <QVBoxLayout>

#include "langselector.h"

#include "reactivebutton.h"

LangSelector::LangSelector(QApplication* application, QWidget* parent) :
    QDialog(parent)
{
    this->application = application;

    this->translator = new QTranslator();
    this->translator->load(":/translations/français/français");

    QVBoxLayout* layout = new QVBoxLayout(this);

    this->title = new QLabel();
    this->title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    this->buttonEnglish = new ReactiveButton("English");
    connect(this->buttonEnglish, SIGNAL(clicked()), this, SLOT(accept()));
    layout->addWidget(this->buttonEnglish);

    this->buttonFrench = new ReactiveButton("Français");
    connect(this->buttonFrench, SIGNAL(mouseEnverEvent()), this, SLOT(setFrench()));
    connect(this->buttonFrench, SIGNAL(mouseLeaveEvent()), this, SLOT(setEnglish()));
    connect(this->buttonFrench, SIGNAL(clicked()), this, SLOT(accept()));
    layout->addWidget(this->buttonFrench);

    retranslateUi();
}

void LangSelector::retranslateUi()
{
    this->title->setText("<b>" + tr("Choose your language") + "</b>");
}

void LangSelector::setEnglish()
{
    this->application->removeTranslator(translator);
    retranslateUi();
}

void LangSelector::setFrench()
{
    this->application->installTranslator(translator);
    retranslateUi();
}

QTranslator* LangSelector::getTranslator()
{
    return this->translator;
}
