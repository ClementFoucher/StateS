/*
 * Copyright © 2014-2017 Clément Foucher
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
#include <QDesktopWidget>
#include <QMessageBox>
#include <QToolBar>
#include <QApplication>
#include <QSplitter>
#include <QAction>
#include <QKeyEvent>
#include <QMimeData>

// StateS classes
#include "states.h"
#include "resourcebar.h"
#include "vhdlexportdialog.h"
#include "imageexportdialog.h"
#include "displayarea.h"
#include "svgimagegenerator.h"
#include "genericscene.h"
#include "fsm.h"
#include "fsmvhdlexport.h"
#include "errordisplaydialog.h"
#include "toolbar.h"
#include "machineconfiguration.h"


StatesUi::StatesUi() :
    QMainWindow(nullptr)
{
    this->setWindowIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/StateS"))));
    this->updateTitle();
    //this->setMaximumSize(QApplication::desktop()->availableGeometry().size());
    this->resize(QApplication::desktop()->availableGeometry().size() - QSize(200, 200));
    this->setAcceptDrops(true);

    // Center window
    this->move(QPoint((QApplication::desktop()->availableGeometry().width()-this->width())/2,
                      (QApplication::desktop()->availableGeometry().height()-this->height())/2
                      )
               );

    // Add scene and resource bar
    QSplitter* splitter = new QSplitter();
    splitter->setOrientation(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    this->setCentralWidget(splitter);

    this->displayArea = new DisplayArea(splitter);
    this->resourceBar = new ResourceBar(splitter);

    connect(this->displayArea, &DisplayArea::itemSelectedEvent,       this, &StatesUi::itemSelectedInSceneEventHandler);
    connect(this->displayArea, &DisplayArea::editSelectedItemEvent,   this, &StatesUi::editSelectedItem);
    connect(this->displayArea, &DisplayArea::renameSelectedItemEvent, this, &StatesUi::renameSelectedItem);

    QList<int> widths;
    // Begin with 2/3 - 1/3
    widths.append( ( 66 * splitter->sizeHint().width() ) / 100 );
    widths.append( ( 33 * splitter->sizeHint().width() ) / 100 );
    splitter->setSizes(widths);

    // Connect tool bar
    ToolBar* toolBar = this->displayArea->getToolbar();

    connect(toolBar, &ToolBar::saveAsRequestedEvent,      this, &StatesUi::beginSaveAsProcedure);
    connect(toolBar, &ToolBar::saveRequestedEvent,        this, &StatesUi::beginSaveProcedure);
    connect(toolBar, &ToolBar::loadRequestedEvent,        this, &StatesUi::beginLoadProcedure);
    connect(toolBar, &ToolBar::newMachineRequestedEvent,  this, &StatesUi::beginNewMachineProcedure);
    connect(toolBar, &ToolBar::exportImageRequestedEvent, this, &StatesUi::beginExportImageProcedure);
    connect(toolBar, &ToolBar::exportHdlRequestedEvent,   this, &StatesUi::beginExportVhdlProcedure);
}

void StatesUi::setMachine(shared_ptr<Machine> newMachine, const QString& path)
{
    if (! this->machine.expired())
    {
        disconnect(this->machine.lock().get(), &Machine::machineUnsavedStateChanged, this, &StatesUi::machineUnsavedStateChangedEventHandler);
    }

    ToolBar* toolBar = this->displayArea->getToolbar();
    if (newMachine != nullptr)
    {
        toolBar->setSaveAsActionEnabled(true);
        //toolBar->setClearActionEnabled(true);
        toolBar->setExportActionsEnabled(true);

        if (path.length() != 0)
            toolBar->setSaveActionEnabled(newMachine->isUnsaved());
        else
            toolBar->setSaveActionEnabled(false);

        connect(newMachine.get(), &Machine::machineUnsavedStateChanged, this, &StatesUi::machineUnsavedStateChangedEventHandler);
    }
    else
    {
        toolBar->setSaveActionEnabled(false);
        //toolBar->setClearActionEnabled(false);
        toolBar->setExportActionsEnabled(false);
        toolBar->setSaveActionEnabled(false);
    }

    this->machine = newMachine;

    this->resourceBar->setMachine(newMachine);
    this->displayArea->setMachine(newMachine);

    this->setCurrentFilePath(path);
}

void StatesUi::setCurrentFilePath(const QString& path)
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        this->currentFilePath = path;

        ToolBar* toolBar = this->displayArea->getToolbar();
        if (path.length() != 0)
            toolBar->setSaveActionEnabled(l_machine->isUnsaved());
        else
            toolBar->setSaveActionEnabled(false);
    }

    this->updateTitle();
}

void StatesUi::setConfiguration(shared_ptr<MachineConfiguration> configuration)
{
    this->displayArea->setZoomLevel(configuration->zoomLevel);
    this->displayArea->setViewCenter(configuration->viewCenter);
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

    if(doClose)
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

    if ( ((event->modifiers() | Qt::CTRL) != 0) && (event->key() == Qt::Key_S) )
    {
        if (! this->currentFilePath.isEmpty())
        {
            emit this->saveMachineInCurrentFileRequestEvent(this->buildConfiguration());
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

    if(transmitEvent)
        QWidget::keyPressEvent(event);
}

void StatesUi::keyReleaseEvent(QKeyEvent* event)
{
    bool transmitEvent = true;

    if ( ((event->modifiers() | Qt::CTRL) != 0) && (event->key() == Qt::Key_S) )
    {
        // As we didn't tranmitted the press event, do the same with the release
        transmitEvent = false;
    }

    if(transmitEvent)
        QWidget::keyPressEvent(event);
}

void StatesUi::beginExportImageProcedure()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        // TODO: Should not act directly on scene
        this->displayArea->getScene()->clearSelection();

        shared_ptr<MachineImageExporter> exporter(new MachineImageExporter(l_machine, this->displayArea->getScene(), this->resourceBar->getComponentVisualizationScene()));

        unique_ptr<ImageExportDialog> exportOptions(new ImageExportDialog(l_machine->getName(), exporter, this->getCurrentDirPath()));
        exportOptions->setModal(true);

        exportOptions->exec();

        if (exportOptions->result() == QDialog::Accepted)
        {
            QString filePath = exportOptions->getFilePath();

            QString comment = tr("Created with") + " StateS v." + StateS::getVersion();
            exporter->doExport(filePath, exportOptions->getImageFormat(), comment);
        }
    }
}

void StatesUi::beginExportVhdlProcedure()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        unique_ptr<FsmVhdlExport> exporter(new FsmVhdlExport(dynamic_pointer_cast<Fsm>(l_machine)));
        shared_ptr<FsmVhdlExport::ExportCompatibility> compat = exporter->checkCompatibility();

        unique_ptr<VhdlExportDialog> exportOptions(new VhdlExportDialog(l_machine->getName(), this->getCurrentDirPath(), !compat->isCompatible()));
        exportOptions->setModal(true);



        exportOptions->exec();

        if (exportOptions->result() == QDialog::Accepted)
        {
            exporter->setOptions(exportOptions->isResetPositive(), exportOptions->prefixIOs());
            exporter->writeToFile(exportOptions->getFilePath());
        }
    }
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

void StatesUi::machineUnsavedStateChangedEventHandler(bool)
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        ToolBar* toolBar = this->displayArea->getToolbar();
        if (this->currentFilePath.length() != 0)
            toolBar->setSaveActionEnabled(l_machine->isUnsaved());
        else
            toolBar->setSaveActionEnabled(false);
    }

    this->updateTitle();
}

void StatesUi::beginSaveAsProcedure()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save machine"), l_machine->getName() + ".SfsmS", "*.SfsmS");

        if (! fileName.isEmpty())
        {
            if (!fileName.endsWith(".SfsmS", Qt::CaseInsensitive))
                fileName += ".SfsmS";

            emit this->saveMachineRequestEvent(fileName, this->buildConfiguration());
        }
    }
}

void StatesUi::beginLoadProcedure()
{
    bool doLoad = this->displayUnsavedConfirmation(tr("Overwrite current machine?"));

    if (doLoad)
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Load machine"), QString(), "*.SfsmS");

        if (! fileName.isEmpty())
        {
            emit loadMachineRequestEvent(fileName);
        }
    }
}

void StatesUi::beginSaveProcedure()
{
    bool doSave = false;

    if (! this->machine.expired())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Update content of file") + " " + this->currentFilePath + " " + tr("with current machine?"), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            doSave = true;
        }
    }

    if (doSave)
    {
        emit saveMachineInCurrentFileRequestEvent(this->buildConfiguration());
    }
}

void StatesUi::updateTitle()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine == nullptr)
    {
        this->setWindowTitle("StateS");
    }
    else
    {
        QString title;

        if (this->currentFilePath != QString::null)
        {
            title = "StateS — " + this->currentFilePath;

        }
        else
        {
            title = "StateS — (" + tr("Unsaved machine") + ")";
        }

        if (l_machine->isUnsaved())
            title += "*";

        this->setWindowTitle(title);
    }
}

shared_ptr<MachineConfiguration> StatesUi::buildConfiguration() const
{
    shared_ptr<MachineConfiguration> configuration(new MachineConfiguration());

    QRectF minimalRect = this->displayArea->getScene()->itemsBoundingRect();

    configuration->sceneTranslation = -minimalRect.topLeft();
    configuration->zoomLevel        = this->displayArea->getZoomLevel();
    configuration->viewCenter       = this->displayArea->getVisibleArea().center();

    return configuration;
}

bool StatesUi::displayUnsavedConfirmation(const QString& cause)
{
    bool userConfirmed = false;

    shared_ptr<Machine> l_machine = this->machine.lock();

    if ( (l_machine != nullptr) && (l_machine->isUnsaved()) )
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), cause + "<br />" + tr("Unsaved changes will be lost."), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            userConfirmed = true;
        }
    }
    else // No machine or saved machine: do not ask, implicit confirmation.
    {
        userConfirmed = true;
    }

    return userConfirmed;
}

QString StatesUi::getCurrentDirPath() const
{
    return this->currentFilePath;
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
    QString fileName = mimeData->urls()[0].path();

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
