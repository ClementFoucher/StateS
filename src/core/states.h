/*
 * Copyright © 2014-2023 Clément Foucher
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

#ifndef STATES_H
#define STATES_H

// Parent
#include <QObject>

// Qt classes
class QApplication;
class QTranslator;

// StateS classes
class StatesUi;
class LangSelectionDialog;


/**
 * @brief The StateS class is the root object of this application.
 *
 * At application launch, it builds builds and displays
 * the language selection dialog before building the UI.
 *
 * During application run, it is in charge of machine replacements
 * (clear, new) and save files (load, save).
 * It is also in charge of displaying errors related to file parsing.
 *
 * This class also gives access to static functions providing
 * information about application version.
 */
class StateS : public QObject
{
	Q_OBJECT

	/////
	// Static functions
public:
	static QString getVersion();
	static QString getCopyrightYears();

	/////
	// Constructors/destructors
public:
	explicit StateS(QApplication* app, const QString& initialFilePath = QString());
	~StateS();

	/////
	// Object functions
private slots:
	// Handle signal from language selection dialog
	void languageSelected(QTranslator* translator);

	// Handle signals from main UI
	void generateNewFsm();
	void clearMachine();

	void loadMachine(const QString& path);

	void saveCurrentMachine(const QString& path);
	void saveCurrentMachineInCurrentFile();

private:
	void launchUi();
	void displayErrorMessages(const QString& errorTitle, const QList<QString>& errorList);
	void displayErrorMessage(const QString& errorTitle, const QString& error);

	/////
	// Object variables
private:
	// Transient member: only used during initialization
	LangSelectionDialog* languageSelectionWindow = nullptr;

	// Pointers to objects persistent throughout the application life
	QTranslator* translator = nullptr; // Translator will be nullptr if English is chosen
	StatesUi* statesUi = nullptr;

};

#endif // STATES_H
