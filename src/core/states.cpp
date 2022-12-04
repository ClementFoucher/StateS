/*
 * Copyright © 2014-2022 Clément Foucher
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
#include "states.h"

// Qt classes
#include <QGuiApplication>
#include <QScreen>
#include <QFileInfo>
#include <QDir>
#include <QTranslator>
#include <QSettings>

// StateS classes
#include "machinemanager.h"
#include "statesui.h"
#include "langselectiondialog.h"
#include "errordisplaydialog.h"
#include "statesexception.h"
#include "viewconfiguration.h"
#include "fsm.h"
#include "machinexmlwriter.h"
#include "machinexmlparser.h"
#include "xmlimportexportbuilder.h"
#include "machinestatus.h"


////
// Static functions for convenience

QString StateS::getVersion()
{
	return STATES_VERSION;
}

QString StateS::getCopyrightYears()
{
	return STATES_YEARS;
}

////
// Object members

/**
 * @brief StateS::StateS
 * Constructor for the main StateS object.
 * Builds the machine manaager and display language selection window.
 * @param initialFilePath
 * Parameter indicating that we have to load an initial machine from file.
 */
StateS::StateS(QApplication* app, const QString& initialFilePath)
{
	// Build machine manager
	this->machineManager = make_shared<MachineManager>();
	this->machineManager->build();

	// Build and show language selection dialog
	this->languageSelectionWindow = new LangSelectionDialog(app);
	connect(this->languageSelectionWindow, &LangSelectionDialog::languageSelected, this, &StateS::languageSelected);

	QSettings windowGeometrySetting("DoubleUnderscore", "StateS");
	QByteArray windowGeometry = windowGeometrySetting.value("LanguageWindowGeometry", QByteArray()).toByteArray();
	if (windowGeometry.isEmpty() == false)
	{
		this->languageSelectionWindow->restoreGeometry(windowGeometry);
	}
	else
	{
		// Set default position: screen center (does not work on Wayland)
		QSize screenSize = QGuiApplication::primaryScreen()->size();
		QSize windowSize = this->languageSelectionWindow->sizeHint();
		this->languageSelectionWindow->move((screenSize.width()  - windowSize.width() )/2,
		                                    (screenSize.height() - windowSize.height())/2
		                                   );
	}

	this->languageSelectionWindow->show();

	// Set initial machine
	if (initialFilePath.isEmpty() == false)
	{
		this->loadMachine(initialFilePath);
	}
	else
	{
		this->generateNewFsm();
	}
}

/**
 * @brief StateS::~StateS
 * Destructor.
 */
StateS::~StateS()
{
	// Save main window geometry
	QSettings windowGeometrySetting("DoubleUnderscore", "StateS");
	QByteArray windowGeometry = this->statesUi->saveGeometry();
	windowGeometrySetting.setValue("MainWindowGeometry", windowGeometry);

	// Force cleaning order to handle dependencies between members
	delete this->statesUi;

	this->machineManager->clear();
	this->machineManager.reset();

	delete this->translator;
}

/**
 * @brief StateS::languageSelected
 * This function is called when language has been chosen and
 * laanguage selection window is about to close.
 * @param translator
 * Pointer to the chosen translator object.
 */
void StateS::languageSelected(QTranslator* translator)
{
	// Store pointer to translator, it will only be deleted on UI close
	this->translator = translator;

	// Store language window geometry
	QSettings windowGeometrySetting("DoubleUnderscore", "StateS");
	QByteArray languageWindowGeometry = this->languageSelectionWindow->saveGeometry();
	windowGeometrySetting.setValue("LanguageWindowGeometry", languageWindowGeometry);

	// Language window self destroys on close, clear pointer
	this->languageSelectionWindow = nullptr;

	// Launch UI
	this->launchUi();
}

/**
 * @brief StateS::generateNewFsm
 * Replaces the existing machine with a newly created FSM.
 * This is the 'Clean' action.
 */
void StateS::generateNewFsm()
{
	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	machineStatus->setHasSaveFile(false);
	machineStatus->setUnsavedFlag(false);

	shared_ptr<Machine> newMachine = shared_ptr<Fsm>(new Fsm());

	this->machineManager->setViewConfiguration(shared_ptr<ViewConfiguration>(new ViewConfiguration()));
	this->machineManager->setMachine(newMachine);
}

/**
 * @brief StateS::clearMachine
 * Clear the currently existing machine.
 * This is the 'Close' action.
 */
void StateS::clearMachine()
{
	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	machineStatus->setHasSaveFile(false);
	machineStatus->setUnsavedFlag(false);

	this->machineManager->setViewConfiguration(nullptr);
	this->machineManager->setMachine(nullptr);
}

/**
 * @brief StateS::loadMachine
 * Loads a machine from a saved file.
 * This is the 'Load' action.
 * @param path
 * Path of file to load from.
 */
void StateS::loadMachine(const QString& path)
{
	QFileInfo file(path);

	if ( (file.exists()) && ( (file.permissions() & QFileDevice::ReadUser) != 0) )
	{
		try
		{
			// Build file parser
			shared_ptr<MachineXmlParser> parser =  XmlImportExportBuilder::buildFileParser(shared_ptr<QFile>(new QFile(path)));

			// Parse and check for warnings
			parser->doParse();
			QList<QString> warnings = parser->getWarnings();
			if (!warnings.isEmpty())
			{
				this->displayErrorMessages(tr("Issues occured reading the file. StateS still managed to load machine."), warnings);
			}

			// If we reached this point, there should have been no exception
			this->machineManager->setViewConfiguration(parser->getViewConfiguration());
			this->machineManager->setMachine(parser->getMachine());

			shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
			machineStatus->setHasSaveFile(true);
			machineStatus->setUnsavedFlag(false);
			machineStatus->setSaveFilePath(path);
		}
		catch (const StatesException& e)
		{
			if (e.getSourceClass() == "FsmSaveFileManager")
			{
				this->displayErrorMessage(tr("Unable to load file."), QString(e.what()));
			}
			else if (e.getSourceClass() == "MachineSaveFileManager")
			{
				this->displayErrorMessage(tr("Unable to load file."), QString(e.what()));
			}
			else
				throw;
		}
	}
}

/**
 * @brief StateS::saveCurrentMachine
 * Saves the current machine to a specified file.
 * This is the 'Save as' action.
 * @param path
 * Path of file to save to.
 */
void StateS::saveCurrentMachine(const QString& path)
{
	if (this->machineManager->getMachine() != nullptr)
	{
		bool fileOk = false;

		QFileInfo file(path);
		if ( (file.exists()) && ( (file.permissions() & QFileDevice::WriteUser) != 0) )
		{
			fileOk = true;
		}
		else if ( (! file.exists()) && (file.absoluteDir().exists()) )
		{
			fileOk = true;
		}

		if (fileOk)
		{
			shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
			machineStatus->setHasSaveFile(true);
			machineStatus->setSaveFilePath(path);
			this->saveCurrentMachineInCurrentFile();
		}
	}
}

/**
 * @brief StateS::saveCurrentMachineInCurrentFile
 * Saves the current machine to currently registered save file.
 * This is the 'Save' action.
 */
void StateS::saveCurrentMachineInCurrentFile()
{
	if (this->machineManager->getMachine() != nullptr)
	{
		bool fileOk = false;
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
		QFileInfo file = QFileInfo(machineStatus->getSaveFileFullPath());
		if ( (file.exists()) && ( (file.permissions() & QFileDevice::WriteUser) != 0) )
		{
			fileOk = true;
		}
		else if ( (! file.exists()) && (file.absoluteDir().exists()) )
		{
			fileOk = true;
		}

		if (fileOk)
		{
			try
			{
				this->machineManager->updateViewConfiguration();
				shared_ptr<MachineXmlWriter> saveManager = XmlImportExportBuilder::buildMachineWriter(this->machineManager);
				saveManager->writeMachineToFile(); // Throws StatesException

				machineStatus->setUnsavedFlag(false);
			}
			catch (const StatesException& e)
			{
				if (e.getSourceClass() == "FsmSaveFileManager")
				{
					this->displayErrorMessage(tr("Unable to save file."), QString(e.what()));
				}
				if (e.getSourceClass() == "MachineSaveFileManager")
				{
					this->displayErrorMessage(tr("Unable to save file."), QString(e.what()));
				}
				else
					throw;
			}
		}
	}
}

/**
 * @brief StateS::launchUi
 * Function called to build and display main UI.
 */
void StateS::launchUi()
{
	// Build main UI
	this->statesUi = new StatesUi(this->machineManager);
	connect(this->statesUi, &StatesUi::newFsmRequestEvent,                   this, &StateS::generateNewFsm);
	connect(this->statesUi, &StatesUi::clearMachineRequestEvent,             this, &StateS::clearMachine);
	connect(this->statesUi, &StatesUi::loadMachineRequestEvent,              this, &StateS::loadMachine);
	connect(this->statesUi, &StatesUi::saveMachineRequestEvent,              this, &StateS::saveCurrentMachine);
	connect(this->statesUi, &StatesUi::saveMachineInCurrentFileRequestEvent, this, &StateS::saveCurrentMachineInCurrentFile);

	// Set UI geometry
	QSettings windowGeometrySetting("DoubleUnderscore", "StateS");
	QByteArray mainWindowGeometry = windowGeometrySetting.value("MainWindowGeometry", QByteArray()).toByteArray();
	if (mainWindowGeometry.isEmpty() == false)
	{
		this->statesUi->restoreGeometry(mainWindowGeometry);
	}
	else
	{
		// Set main window default size and position
		QSize screenSize = QGuiApplication::primaryScreen()->size();

		// Set the window to cover 85% of the screen
		this->statesUi->resize(85*screenSize/100);

		// Center window (does not work on Wayland)
		this->statesUi->move((screenSize.width()  - this->statesUi->width() )/2,
		                     (screenSize.height() - this->statesUi->height())/2
		                    );
	}

	// Display UI
	this->statesUi->show();
}

/**
 * @brief StateS::displayErrorMessages
 * Displays a list of error messages in a modal window.
 * @param errorTitle
 * Tite to display above errors list.
 * @param errorList
 * List of error messages to display.
 */
void StateS::displayErrorMessages(const QString& errorTitle, const QList<QString>& errorList)
{
	QWidget* parent = nullptr;
	if (this->statesUi != nullptr)
	{
		parent = this->statesUi;
	}
	else if (this->languageSelectionWindow != nullptr)
	{
		parent = this->languageSelectionWindow;
	}
	ErrorDisplayDialog* errorDialog = new ErrorDisplayDialog(errorTitle, errorList, parent);
	errorDialog->setModal(true);
	connect(errorDialog, &ErrorDisplayDialog::accepted, errorDialog, &ErrorDisplayDialog::deleteLater);
	errorDialog->open();
}

/**
 * @brief StateS::displayErrorMessage
 * Displays a single error message in a modal window.
 * @param errorTitle
 * Tite to display above error message.
 * @param error
 * Error message to display.
 */
void StateS::displayErrorMessage(const QString& errorTitle, const QString& error)
{
	QWidget* parent = nullptr;
	if (this->statesUi != nullptr)
	{
		parent = this->statesUi;
	}
	else if (this->languageSelectionWindow != nullptr)
	{
		parent = this->languageSelectionWindow;
	}
	ErrorDisplayDialog* errorDialog = new ErrorDisplayDialog(errorTitle, error, parent);
	errorDialog->setModal(true);
	connect(errorDialog, &ErrorDisplayDialog::accepted, errorDialog, &ErrorDisplayDialog::deleteLater);
	errorDialog->open();
}
