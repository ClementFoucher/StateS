/*
 * Copyright © 2014-2021 Clément Foucher
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
#include "statesui.h"

// Qt classes
#include <QFileDialog>
#include <QMessageBox>
#include <QGuiApplication>
#include <QSplitter>
#include <QKeyEvent>
#include <QMimeData>
#include <QScreen>

// StateS classes
#include "states.h"
#include "machinemanager.h"
#include "maintoolbar.h"
#include "displayarea.h"
#include "resourcebar.h"
#include "vhdlexportdialog.h"
#include "imageexportdialog.h"
#include "fsmvhdlexport.h"
#include "svgimagegenerator.h"
#include "fsm.h"
#include "errordisplaydialog.h"
#include "machinestatus.h"
#include "machineeditorwidget.h"
#include "timelinewidget.h"


StatesUi::StatesUi(shared_ptr<MachineManager> machineManager)
{
	this->machineManager = machineManager;
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent,               this, &StatesUi::machineUpdatedEventHandler);
	connect(machineManager.get(), &MachineManager::undoActionAvailabilityChangeEvent, this, &StatesUi::undoActionAvailabilityChangeEventHandler);
	connect(machineManager.get(), &MachineManager::redoActionAvailabilityChangeEvent, this, &StatesUi::redoActionAvailabilityChangeEventHandler);

	shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
	connect(machineStatus.get(), &MachineStatus::saveFilePathChangedEvent, this, &StatesUi::machineFilePathUpdated);
	connect(machineStatus.get(), &MachineStatus::unsavedFlagChangedEvent,  this, &StatesUi::machineUnsavedStateUpdated);

	/***
	 * Build window
	 ***/

	this->setWindowIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/StateS"))));
	this->updateTitle();

	// Set main window size and position
	QSize screenSize = QGuiApplication::primaryScreen()->size();

	// Set the window to cover 85% of the screen
	this->resize(screenSize - 15*screenSize/100);

	// Center window
	this->move(QPoint((screenSize.width()-this->width())/2,
	                  (screenSize.height()-this->height())/2
	                  )
	           );

	// Allow dropping on window
	this->setAcceptDrops(true);

	/***
	 * Build window content
	 ***/
	QSplitter* splitter = new QSplitter(this);
	splitter->setOrientation(Qt::Horizontal);
	splitter->setChildrenCollapsible(false);
	this->setCentralWidget(splitter);

	this->displayArea = new DisplayArea(splitter);
	this->resourceBar = new ResourceBar(this->machineManager, splitter);

	this->toolbar = new MainToolBar(this->displayArea);
	this->displayArea->setToolBar(this->toolbar);

	this->editor = new MachineEditorWidget(this->machineManager, this->displayArea);
	this->displayArea->addWidget(this->editor, tr("Machine"));

	// Begin view with 2/3 scene - 1/3 resource bar
	QList<int> widths;
	widths.append( ( 66 * splitter->sizeHint().width() ) / 100 );
	widths.append( ( 33 * splitter->sizeHint().width() ) / 100 );
	splitter->setSizes(widths);

	/***
	 * Connect signals
	 ***/

	// Tool bar
	connect(this->toolbar, &MainToolBar::saveAsRequestedEvent,      this, &StatesUi::beginSaveAsProcedure);
	connect(this->toolbar, &MainToolBar::saveRequestedEvent,        this, &StatesUi::beginSaveProcedure);
	connect(this->toolbar, &MainToolBar::loadRequestedEvent,        this, &StatesUi::beginLoadProcedure);
	connect(this->toolbar, &MainToolBar::newMachineRequestedEvent,  this, &StatesUi::beginNewMachineProcedure);
	connect(this->toolbar, &MainToolBar::exportImageRequestedEvent, this, &StatesUi::beginExportImageProcedure);
	connect(this->toolbar, &MainToolBar::exportHdlRequestedEvent,   this, &StatesUi::beginExportVhdlProcedure);
	connect(this->toolbar, &MainToolBar::undo,                      this, &StatesUi::undo);
	connect(this->toolbar, &MainToolBar::redo,                      this, &StatesUi::redo);

	// Display area
	connect(this->editor, &MachineEditorWidget::itemSelectedEvent,       this, &StatesUi::itemSelectedInSceneEventHandler);
	connect(this->editor, &MachineEditorWidget::editSelectedItemEvent,   this, &StatesUi::editSelectedItem);
	connect(this->editor, &MachineEditorWidget::renameSelectedItemEvent, this, &StatesUi::renameSelectedItem);
}

void StatesUi::resetUi()
{
	this->updateTitle();
	if (this->machineManager->getMachine() != nullptr)
	{
		this->toolbar->setSaveAsActionEnabled(true);
		this->toolbar->setExportActionsEnabled(true);
	}
	else
	{
		this->toolbar->setSaveAsActionEnabled(false);
		this->toolbar->setExportActionsEnabled(false);
	}

	this->toolbar->setUndoActionEnabled(false);
	this->toolbar->setRedoActionEnabled(false);
	this->toolbar->setSaveActionEnabled(false);
}

void StatesUi::setViewConfiguration(shared_ptr<ViewConfiguration> configuration)
{
	this->editor->setViewConfiguration(configuration);
}

void StatesUi::beginNewMachineProcedure()
{
	bool doNew = this->displayUnsavedConfirmation(tr("Clear current machine?"));

	if (doNew)
		emit newFsmRequestEvent();
}

void StatesUi::beginClearMachineProcedure()
{
	bool doClear = this->displayUnsavedConfirmation(tr("Delete current machine?"));

	if (doClear)
		emit clearMachineRequestEvent();
}

void StatesUi::closeEvent(QCloseEvent* event)
{
	bool doClose = this->displayUnsavedConfirmation(tr("Quit StateS?"));

	if (doClose)
	{
		// When main window closing, we should terminate application.
		// This is not done automatically if another window is open.

		// Setting machine to nullptr clears child widgets,
		// closing such windows.
		emit clearMachineRequestEvent();

		QWidget::closeEvent(event);
	}
	else
	{
		event->ignore();
	}
}

void StatesUi::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if ( ((event->modifiers() & Qt::CTRL) != 0) && (event->key() == Qt::Key_S) )
	{
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
		if (machineStatus->getHasSaveFile() == true)
		{
			emit this->saveMachineInCurrentFileRequestEvent();
			// Should make button blink for one second.
			// How to without locking UI?
		}
		else
		{
			// No current file, trigger 'save as' procedure
			this->beginSaveAsProcedure();
		}

		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && ((event->modifiers() & Qt::SHIFT) == 0) && (event->key() == Qt::Key_Z) )
	{
		shared_ptr<Machine> l_machine = this->machineManager->getMachine();
		if ( (l_machine != nullptr) && (l_machine->getCurrentSimulationMode() == Machine::simulation_mode::editMode) )
		{
			this->undo();
		}
		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && (event->key() == Qt::Key_Y) )
	{
		shared_ptr<Machine> l_machine = this->machineManager->getMachine();
		if ( (l_machine != nullptr) && (l_machine->getCurrentSimulationMode() == Machine::simulation_mode::editMode) )
		{
			this->redo();
		}
		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && ((event->modifiers() & Qt::SHIFT) != 0) && (event->key() == Qt::Key_Z) )
	{
		shared_ptr<Machine> l_machine = this->machineManager->getMachine();
		if ( (l_machine != nullptr) && (l_machine->getCurrentSimulationMode() == Machine::simulation_mode::editMode) )
		{
			this->redo();
		}
		transmitEvent = false;
	}

	if (transmitEvent)
	{
		QWidget::keyPressEvent(event);
	}
}

void StatesUi::keyReleaseEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	// As we didn't transmitted the press event, do the same with the release
	if ( ((event->modifiers() & Qt::CTRL) != 0) && (event->key() == Qt::Key_S) )
	{
		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && ((event->modifiers() & Qt::SHIFT) == 0) && (event->key() == Qt::Key_Z) )
	{
		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && (event->key() == Qt::Key_Y) )
	{
		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && ((event->modifiers() & Qt::SHIFT) != 0) && (event->key() == Qt::Key_Z) )
	{
		transmitEvent = false;
	}

	if (transmitEvent)
	{
		QWidget::keyPressEvent(event);
	}
}

void StatesUi::beginExportImageProcedure()
{
	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if (l_machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();

		this->editor->clearSelection();

		shared_ptr<MachineImageExporter> exporter(new MachineImageExporter(l_machine, this->editor->getScene(), this->resourceBar->getComponentVisualizationScene()));

		unique_ptr<ImageExportDialog> exportOptions(new ImageExportDialog(l_machine->getName(), exporter, machineStatus->getImageExportPath(), this));
		exportOptions->setModal(true);

		exportOptions->exec();

		if (exportOptions->result() == QDialog::Accepted)
		{
			QString filePath = exportOptions->getFilePath();

			QString comment = tr("Created with") + " StateS v." + StateS::getVersion();
			exporter->doExport(filePath, exportOptions->getImageFormat(), comment);

			machineStatus->setImageExportPath(filePath);
		}
	}
}

void StatesUi::beginExportVhdlProcedure()
{
	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if (l_machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();

		unique_ptr<FsmVhdlExport> exporter(new FsmVhdlExport(dynamic_pointer_cast<Fsm>(l_machine)));
		shared_ptr<FsmVhdlExport::ExportCompatibility> compat = exporter->checkCompatibility();

		unique_ptr<VhdlExportDialog> exportOptions(new VhdlExportDialog(l_machine->getName(), machineStatus->getVhdlExportPath(), !compat->isCompatible(), this));
		exportOptions->setModal(true);

		exportOptions->exec();

		if (exportOptions->result() == QDialog::Accepted)
		{
			QString filePath = exportOptions->getFilePath();

			exporter->setOptions(exportOptions->isResetPositive(), exportOptions->prefixIOs());
			exporter->writeToFile(filePath);

			machineStatus->setVhdlExportPath(filePath);
		}
	}
}

void StatesUi::undo()
{
	this->machineManager->undo();
}

void StatesUi::redo()
{
	this->machineManager->redo();
}

void StatesUi::itemSelectedInSceneEventHandler(shared_ptr<MachineComponent> item)
{
	this->resourceBar->setSelectedItem(item);
}

void StatesUi::editSelectedItem()
{
	this->resourceBar->editSelectedItem();
}

void StatesUi::renameSelectedItem()
{
	this->resourceBar->renameSelectedItem();
}

void StatesUi::machineFilePathUpdated()
{
	this->updateTitle();

	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	if (machineStatus->getHasSaveFile() == true)
	{
		this->toolbar->setSaveActionEnabled(machineStatus->getUnsavedFlag());
	}
}

void StatesUi::machineUnsavedStateUpdated()
{
	this->updateTitle();

	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	if (machineStatus->getHasSaveFile() == true)
	{
		this->toolbar->setSaveActionEnabled(machineStatus->getUnsavedFlag());
	}
}

void StatesUi::simulationModeToggledEventHandler(Machine::simulation_mode newMode)
{
	// TODO: moving states is allowed during simlulation, which can trigger undo availability.
	// These should be obtained from the undo stack instead of just saved here.
	static bool isUndoEnabled;
	static bool isRedoEnabled;

	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if ( (l_machine != nullptr) && (newMode == Machine::simulation_mode::simulateMode) )
	{
		this->timeline = new TimelineWidget(l_machine, this);
		connect(this->timeline, &TimelineWidget::detachTimelineEvent, this, &StatesUi::setTimelineDetachedState);

		this->displayArea->addWidget(this->timeline, tr("Timeline"));

		isUndoEnabled = this->toolbar->getUndoActionEnabled();
		isRedoEnabled = this->toolbar->getRedoActionEnabled();

		this->toolbar->setNewFsmActionEnabled(false);
		this->toolbar->setUndoActionEnabled(false);
		this->toolbar->setRedoActionEnabled(false);
	}
	else
	{
		if (this->timeline != nullptr)
		{
			this->displayArea->removeWidget(this->timeline);
			delete this->timeline;
			this->timeline = nullptr;
		}

		this->toolbar->setNewFsmActionEnabled(true);
		this->toolbar->setUndoActionEnabled(isUndoEnabled);
		this->toolbar->setRedoActionEnabled(isRedoEnabled);
	}
}

void StatesUi::setTimelineDetachedState(bool detach)
{
	if (this->timeline != nullptr)
	{
		if (detach)
		{
			this->displayArea->removeWidget(this->timeline);
			this->timeline->show();
		}
		else
		{
			this->displayArea->addWidget(this->timeline, tr("Timeline"));
		}
	}
}

void StatesUi::undoActionAvailabilityChangeEventHandler(bool undoAvailable)
{
	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if ( (l_machine != nullptr) && (l_machine->getCurrentSimulationMode() == Machine::simulation_mode::editMode) )
	{
		this->toolbar->setUndoActionEnabled(undoAvailable);
	}
}

void StatesUi::redoActionAvailabilityChangeEventHandler(bool redoAvailable)
{
	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if ( (l_machine != nullptr) && (l_machine->getCurrentSimulationMode() == Machine::simulation_mode::editMode) )
	{
		this->toolbar->setRedoActionEnabled(redoAvailable);
	}
}

void StatesUi::machineUpdatedEventHandler(bool isNewMachine)
{
	// Connect signals
	shared_ptr<Machine> newMachine = this->machineManager->getMachine();
	if (newMachine != nullptr)
	{
		this->machineManager->addConnection(connect(newMachine.get(), &Machine::simulationModeChangedEvent, this, &StatesUi::simulationModeToggledEventHandler));
	}

	if (isNewMachine == true)
	{
		this->resetUi();
	}
}

void StatesUi::beginSaveAsProcedure()
{
	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if (l_machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();

		// Try to build the best possible file name
		// depending on current knowledge of paths
		QString filePath;
		if (machineStatus->getHasSaveFile() == true)
		{
			filePath = machineStatus->getSaveFileFullPath();
		}
		else
		{
			QString fileName = l_machine->getName() + ".SfsmS";

			if (machineStatus->getSaveFilePath().isEmpty() == false)
			{
				filePath = QFileInfo(machineStatus->getSaveFilePath() + "/" + fileName).filePath();
			}
			else
			{
				filePath += fileName;
			}
		}

		QString finalFilePath = QFileDialog::getSaveFileName(this, tr("Save machine"), filePath, "*.SfsmS");

		if (! finalFilePath.isEmpty())
		{
			if (!finalFilePath.endsWith(".SfsmS", Qt::CaseInsensitive))
				finalFilePath += ".SfsmS";

			emit this->saveMachineRequestEvent(finalFilePath);
		}
	}
}

void StatesUi::beginLoadProcedure()
{
	bool doLoad = this->displayUnsavedConfirmation(tr("Discard current machine?"));

	if (doLoad)
	{
		QString filePath;
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
		filePath = machineStatus->getSaveFilePath();

		QString finalFilePath = QFileDialog::getOpenFileName(this, tr("Load machine"), filePath, "*.SfsmS");

		if (! finalFilePath.isEmpty())
		{
			emit loadMachineRequestEvent(finalFilePath);
		}
	}
}

void StatesUi::beginSaveProcedure()
{
	emit saveMachineInCurrentFileRequestEvent();
}

void StatesUi::updateTitle()
{
	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if (l_machine == nullptr)
	{
		this->setWindowTitle("StateS");
	}
	else
	{
		QString title;
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
		if (machineStatus->getHasSaveFile() == false)
		{
			title = "StateS — (" + tr("Unsaved machine") + ")";
		}
		else
		{
			title = "StateS — " + machineStatus->getSaveFileFullPath();
		}

		if (machineStatus->getUnsavedFlag() == true)
		{
			title += "*";
		}

		this->setWindowTitle(title);
	}
}

shared_ptr<ViewConfiguration> StatesUi::getViewConfiguration() const
{
	return this->editor->getViewConfiguration();
}

bool StatesUi::displayUnsavedConfirmation(const QString& cause)
{
	bool userConfirmed = false;

	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
	if (l_machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
		if (machineStatus->getUnsavedFlag() == true)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("User confirmation needed"), cause + "<br />" + tr("Unsaved changes will be lost."), QMessageBox::Ok | QMessageBox::Cancel);

			if (reply == QMessageBox::StandardButton::Ok)
			{
				userConfirmed = true;
			}
		}
		else // Saved machine: do not ask, implicit confirmation. // TODO: check if not reversed
		{
			userConfirmed = true;
		}
	}
	else // No machine: do not ask, implicit confirmation. // TODO: check if not reversed
	{
		userConfirmed = true;
	}

	return userConfirmed;
}

void StatesUi::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		if (mimeData->urls().count() == 1)
		{
			event->acceptProposedAction();
		}
	}
}

void StatesUi::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();
	QString fileName = mimeData->urls().at(0).path();

	bool userConfirmed = this->displayUnsavedConfirmation(tr("Load file") + " " + fileName + tr("?") );

	if (userConfirmed == true)
	{
		emit loadMachineRequestEvent(fileName);
	}
}

void StatesUi::displayErrorMessage(const QString& errorTitle, const QList<QString>& errorList)
{
	unique_ptr<ErrorDisplayDialog> errorDialog = unique_ptr<ErrorDisplayDialog>(new ErrorDisplayDialog(errorTitle, errorList, this));
	errorDialog->setModal(true);
	errorDialog->exec();
}

void StatesUi::displayErrorMessage(const QString& errorTitle, const QString& error)
{
	unique_ptr<ErrorDisplayDialog> errorDialog = unique_ptr<ErrorDisplayDialog>(new ErrorDisplayDialog(errorTitle, error, this));
	errorDialog->setModal(true);
	errorDialog->exec();
}
