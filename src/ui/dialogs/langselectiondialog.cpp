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
#include "langselectiondialog.h"

// Qt classes
#include <QApplication>
#include <QTranslator>
#include <QVBoxLayout>
#include <QLabel>

// StateS classes
#include "reactivebutton.h"


LangSelectionDialog::LangSelectionDialog(QApplication* application, QWidget* parent) :
	StatesMainWindow(parent)
{
	// Buid members
	this->application = application;

	this->frenchTranslator = new QTranslator();
	static_cast<void>(this->frenchTranslator->load(QLocale::French, ":/translations/french")); // Discarding the result of this nodiscard function as it is an embedded resource thus we are absolutely sure it can be loaded

	// Build window
	this->setWindowTitle("StateS");
	this->setAttribute(Qt::WA_DeleteOnClose);

	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout(mainWidget);
	this->setCentralWidget(mainWidget);

	this->mainLabel = new QLabel(this);
	this->mainLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(this->mainLabel);

	ReactiveButton* buttonEnglish = new ReactiveButton("English", this);
	connect(buttonEnglish, &ReactiveButton::mouseEnterEvent,      this, &LangSelectionDialog::setEnglish);
	connect(buttonEnglish, &ReactiveButton::keyboardFocusInEvent, this, &LangSelectionDialog::setEnglish);
	connect(buttonEnglish, &QAbstractButton::clicked,             this, &LangSelectionDialog::close);
	layout->addWidget(buttonEnglish);

	ReactiveButton* buttonFrench = new ReactiveButton("Français", this);
	connect(buttonFrench, &ReactiveButton::mouseEnterEvent,      this, &LangSelectionDialog::setFrench);
	connect(buttonFrench, &ReactiveButton::keyboardFocusInEvent, this, &LangSelectionDialog::setFrench);
	connect(buttonFrench, &QAbstractButton::clicked,             this, &LangSelectionDialog::close);
	layout->addWidget(buttonFrench);

	this->retranslateUi();
}

LangSelectionDialog::~LangSelectionDialog()
{
	// Only delete the translator if it isn't being used
	if (this->activeTranslator == nullptr)
	{
		delete this->frenchTranslator;
	}
}

void LangSelectionDialog::closeEvent(QCloseEvent* event)
{
	emit this->languageSelected(this->activeTranslator);
	QMainWindow::closeEvent(event);
}

void LangSelectionDialog::setEnglish()
{
	this->application->removeTranslator(this->frenchTranslator);
	this->activeTranslator = nullptr;
	this->retranslateUi();
}

void LangSelectionDialog::setFrench()
{
	this->application->installTranslator(this->frenchTranslator);
	this->activeTranslator = this->frenchTranslator;
	this->retranslateUi();
}

void LangSelectionDialog::retranslateUi()
{
	this->mainLabel->setText("<b>" + tr("Choose your language") + "</b>");
}
