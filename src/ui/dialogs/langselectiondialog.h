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

#ifndef LANGSELECTIONDIALOG_H
#define LANGSELECTIONDIALOG_H

// Parent
#include "statesmainwindow.h"

// Qt classes
class QLabel;
class QApplication;
class QTranslator;


class LangSelectionDialog : public StatesMainWindow
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit LangSelectionDialog(QApplication* application, QWidget* parent = nullptr);
	~LangSelectionDialog();

	/////
	// Object functions
signals:
	void languageSelected(QTranslator* translator);

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private slots:
	void setEnglish();
	void setFrench();

private:
	void retranslateUi();

	/////
	// Object variables
private:
	QTranslator*  frenchTranslator = nullptr;
	QTranslator*  activeTranslator = nullptr;
	QApplication* application      = nullptr;
	QLabel*       mainLabel        = nullptr;

};

#endif // LANGSELECTIONDIALOG_H
