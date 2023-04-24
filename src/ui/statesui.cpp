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

// Current class header
#include "statesui.h"

// Qt classes
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QKeyEvent>
#include <QMimeData>

// StateS classes
#include "states.h"
#include "machinemanager.h"
#include "machine.h"
#include "maintoolbar.h"
#include "displayarea.h"
#include "resourcebar.h"
#include "vhdlexportdialog.h"
#include "imageexportdialog.h"
#include "fsmvhdlexport.h"
#include "pixmapgenerator.h"
#include "machinestatus.h"
#include "machineeditorwidget.h"
#include "timelinewidget.h"
#include "machineimageexporter.h"


StatesUi::StatesUi()
{
	connect(machineManager.get(), &MachineManager::machineReplacedEvent,               this, &StatesUi::machineReplacedEventHandler);
	connect(machineManager.get(), &MachineManager::undoActionAvailabilityChangedEvent, this, &StatesUi::undoActionAvailabilityChangeEventHandler);
	connect(machineManager.get(), &MachineManager::redoActionAvailabilityChangedEvent, this, &StatesUi::redoActionAvailabilityChangeEventHandler);
	connect(machineManager.get(), &MachineManager::simulationModeChangedEvent,         this, &StatesUi::simulationModeToggledEventHandler);

	shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
	connect(machineStatus.get(), &MachineStatus::saveFilePathChangedEvent, this, &StatesUi::machineFilePathUpdated);
	connect(machineStatus.get(), &MachineStatus::unsavedFlagChangedEvent,  this, &StatesUi::machineUnsavedStateUpdated);

	/***
	 * Build window
	 ***/

	this->setWindowIcon(QIcon(PixmapGenerator::getStatesWindowIcon()));
	this->updateTitle();

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
	this->resourceBar = new ResourceBar(splitter);

	this->toolbar = new MainToolBar(this->displayArea);
	this->displayArea->setToolBar(this->toolbar);

	this->editor = new MachineEditorWidget(this->displayArea);
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

	// Set initial UI state
	this->resetUi();
}

void StatesUi::setView(shared_ptr<ViewConfiguration> viewConfiguration)
{
	this->editor->setView(viewConfiguration);
}

shared_ptr<ViewConfiguration> StatesUi::getView() const
{
	return this->editor->getView();
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
		shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
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
		if (machineManager->getCurrentSimulationMode() == SimulationMode_t::editMode)
		{
			this->undo();
		}
		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && (event->key() == Qt::Key_Y) )
	{
		if (machineManager->getCurrentSimulationMode() == SimulationMode_t::editMode)
		{
			this->redo();
		}
		transmitEvent = false;
	}
	else if ( ((event->modifiers() & Qt::CTRL) != 0) && ((event->modifiers() & Qt::SHIFT) != 0) && (event->key() == Qt::Key_Z) )
	{
		if (machineManager->getCurrentSimulationMode() == SimulationMode_t::editMode)
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

void StatesUi::machineReplacedEventHandler()
{
	this->resetUi();
}

void StatesUi::beginSaveAsProcedure()
{
	auto machine = machineManager->getMachine();
	if (machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();

		// Try to build the best possible file name
		// depending on current knowledge of paths
		QString filePath;
		if (machineStatus->getHasSaveFile() == true)
		{
			filePath = machineStatus->getSaveFileFullPath();
		}
		else
		{
			QString fileName = machine->getName() + ".SfsmS";

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

void StatesUi::beginSaveProcedure()
{
	emit saveMachineInCurrentFileRequestEvent();
}

void StatesUi::beginLoadProcedure()
{
	bool doLoad = this->displayUnsavedConfirmation(tr("Discard current machine?"));

	if (doLoad)
	{
		QString filePath;
		shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
		filePath = machineStatus->getSaveFilePath();

		QString finalFilePath = QFileDialog::getOpenFileName(this, tr("Load machine"), filePath, "*.SfsmS");

		if (! finalFilePath.isEmpty())
		{
			emit loadMachineRequestEvent(finalFilePath);
		}
	}
}

void StatesUi::beginNewMachineProcedure()
{
	bool doNew = this->displayUnsavedConfirmation(tr("Clear current machine?"));

	if (doNew)
	{
		emit newFsmRequestEvent();
	}
}

void StatesUi::beginClearMachineProcedure()
{
	bool doClear = this->displayUnsavedConfirmation(tr("Delete current machine?"));

	if (doClear)
	{
		emit clearMachineRequestEvent();
	}
}

void StatesUi::beginExportImageProcedure()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();

	this->editor->clearSelection();

	shared_ptr<MachineImageExporter> exporter(new MachineImageExporter(this->editor->getScene(), this->resourceBar->getComponentVisualizationScene()));

	unique_ptr<ImageExportDialog> exportOptions(new ImageExportDialog(machine->getName(), exporter, machineStatus->getImageExportPath(), this));
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

void StatesUi::beginExportVhdlProcedure()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();

	unique_ptr<FsmVhdlExport> exporter(new FsmVhdlExport());
	shared_ptr<FsmVhdlExport::ExportCompatibility> compat = exporter->checkCompatibility();

	unique_ptr<VhdlExportDialog> exportOptions(new VhdlExportDialog(machine->getName(), machineStatus->getVhdlExportPath(), !compat->isCompatible(), this));
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

void StatesUi::undo()
{
	machineManager->undo();
}

void StatesUi::redo()
{
	machineManager->redo();
}

void StatesUi::itemSelectedInSceneEventHandler(componentId_t componentId)
{
	this->resourceBar->setSelectedItem(componentId);
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

	shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
	if (machineStatus->getHasSaveFile() == true)
	{
		this->toolbar->setSaveActionEnabled(machineStatus->getUnsavedFlag());
	}
}

void StatesUi::machineUnsavedStateUpdated()
{
	this->updateTitle();

	shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
	if (machineStatus->getHasSaveFile() == true)
	{
		this->toolbar->setSaveActionEnabled(machineStatus->getUnsavedFlag());
	}
}

void StatesUi::simulationModeToggledEventHandler(SimulationMode_t newMode)
{
	static bool isUndoEnabled;
	static bool isRedoEnabled;

	auto machine = machineManager->getMachine();
	if ( (machine != nullptr) && (newMode == SimulationMode_t::simulateMode) )
	{
		this->timeline = new TimelineWidget(this);
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
	if (machineManager->getCurrentSimulationMode() == SimulationMode_t::editMode)
	{
		this->toolbar->setUndoActionEnabled(undoAvailable);
	}
}

void StatesUi::redoActionAvailabilityChangeEventHandler(bool redoAvailable)
{
	if (machineManager->getCurrentSimulationMode() == SimulationMode_t::editMode)
	{
		this->toolbar->setRedoActionEnabled(redoAvailable);
	}
}

void StatesUi::resetUi()
{
	this->updateTitle();
	if (machineManager->getMachine() != nullptr)
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

void StatesUi::updateTitle()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr)
	{
		this->setWindowTitle("StateS");
	}
	else
	{
		QString title;
		shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
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

bool StatesUi::displayUnsavedConfirmation(const QString& cause)
{
	bool userConfirmed = false;

	auto machine = machineManager->getMachine();
	if (machine != nullptr)
	{
		shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
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
