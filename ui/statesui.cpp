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
#include "scenewidget.h"


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
    this->toolbar     = this->displayArea->getToolbar();
    this->sceneWidget = this->displayArea->getSceneWidget();

    connect(this->sceneWidget, &SceneWidget::itemSelectedEvent,       this, &StatesUi::itemSelectedInSceneEventHandler);
    connect(this->sceneWidget, &SceneWidget::editSelectedItemEvent,   this, &StatesUi::editSelectedItem);
    connect(this->sceneWidget, &SceneWidget::renameSelectedItemEvent, this, &StatesUi::renameSelectedItem);

    // Begin view with 2/3 scene - 1/3 resource bar
    QList<int> widths;
    widths.append( ( 66 * splitter->sizeHint().width() ) / 100 );
    widths.append( ( 33 * splitter->sizeHint().width() ) / 100 );
    splitter->setSizes(widths);

    // Connect tool bar
    connect(this->toolbar, &ToolBar::saveAsRequestedEvent,      this, &StatesUi::beginSaveAsProcedure);
    connect(this->toolbar, &ToolBar::saveRequestedEvent,        this, &StatesUi::beginSaveProcedure);
    connect(this->toolbar, &ToolBar::loadRequestedEvent,        this, &StatesUi::beginLoadProcedure);
    connect(this->toolbar, &ToolBar::newMachineRequestedEvent,  this, &StatesUi::beginNewMachineProcedure);
    connect(this->toolbar, &ToolBar::exportImageRequestedEvent, this, &StatesUi::beginExportImageProcedure);
    connect(this->toolbar, &ToolBar::exportHdlRequestedEvent,   this, &StatesUi::beginExportVhdlProcedure);
    connect(this->toolbar, &ToolBar::undo,                      this, &StatesUi::undoRequestEvent);
    connect(this->toolbar, &ToolBar::redo,                      this, &StatesUi::redoRequestEvent);
}

void StatesUi::setMachine(shared_ptr<Machine> newMachine, bool maintainView)
{
    this->machine = newMachine;

    this->resourceBar->setMachine(newMachine, maintainView);
    this->displayArea->setMachine(newMachine);
    this->sceneWidget->setMachine(newMachine, maintainView);
}

void StatesUi::setTitle(const QString& title)
{
    this->windowTitle = title;
    this->updateTitle();
}

void StatesUi::setUnsavedFlag(bool unsaved)
{
    if (this->unsavedFlag != unsaved)
    {
        this->unsavedFlag = unsaved;
        this->updateTitle();
    }
}

void StatesUi::setConfiguration(shared_ptr<MachineConfiguration> configuration)
{
    if (configuration != nullptr)
    {
        this->sceneWidget->setZoomLevel(configuration->zoomLevel);
        this->sceneWidget->centerOn(configuration->viewCenter);
    }
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

    if ( ((event->modifiers() & Qt::CTRL) != 0) && (event->key() == Qt::Key_S) )
    {
        if (! this->windowTitle.isEmpty())
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
        emit this->undoRequestEvent();
        transmitEvent = false;
    }
    else if ( ((event->modifiers() & Qt::CTRL) != 0) && (event->key() == Qt::Key_Y) )
    {
        emit this->redoRequestEvent();
        transmitEvent = false;
    }
    else if ( ((event->modifiers() & Qt::CTRL) != 0) && ((event->modifiers() & Qt::SHIFT) != 0) && (event->key() == Qt::Key_Z) )
    {
        emit this->redoRequestEvent();
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
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        this->sceneWidget->getScene()->clearSelection();

        shared_ptr<MachineImageExporter> exporter(new MachineImageExporter(l_machine, this->sceneWidget->getScene(), this->resourceBar->getComponentVisualizationScene()));

        unique_ptr<ImageExportDialog> exportOptions(new ImageExportDialog(l_machine->getName(), exporter, this->windowTitle));
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

        unique_ptr<VhdlExportDialog> exportOptions(new VhdlExportDialog(l_machine->getName(), this->windowTitle, !compat->isCompatible()));
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

            emit this->saveMachineRequestEvent(fileName);
        }
    }
}

void StatesUi::beginLoadProcedure()
{
    bool doLoad = this->displayUnsavedConfirmation(tr("Discard current machine?"));

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
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Update content of file") + " " + this->windowTitle + " " + tr("with current machine?"), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            doSave = true;
        }
    }

    if (doSave)
    {
        emit saveMachineInCurrentFileRequestEvent();
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

        if (this->windowTitle != QString::null)
        {
            title = "StateS — " + this->windowTitle;

        }
        else
        {
            title = "StateS — (" + tr("Unsaved machine") + ")";
        }

        if (this->unsavedFlag == true)
        {
            title += "*";
        }

        this->setWindowTitle(title);
    }
}

shared_ptr<MachineConfiguration> StatesUi::getConfiguration() const
{
    shared_ptr<MachineConfiguration> configuration(new MachineConfiguration());

    configuration->sceneTranslation = -(this->sceneWidget->getScene()->itemsBoundingRect().topLeft());
    configuration->zoomLevel        = this->sceneWidget->getZoomLevel();
    configuration->viewCenter       = this->sceneWidget->getVisibleArea().center();

    return configuration;
}

bool StatesUi::displayUnsavedConfirmation(const QString& cause)
{
    bool userConfirmed = false;

    shared_ptr<Machine> l_machine = this->machine.lock();

    if ( (l_machine != nullptr) && (this->unsavedFlag == true) )
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

void StatesUi::setSaveAsActionEnabled(bool enable)
{
    this->toolbar->setSaveAsActionEnabled(enable);
}

void StatesUi::setSaveActionEnabled(bool enable)
{
    this->toolbar->setSaveActionEnabled(enable);
}

void StatesUi::setExportActionsEnabled(bool enable)
{
    this->toolbar->setExportActionsEnabled(enable);
}

void StatesUi::setUndoButtonEnabled(bool enable)
{
    this->toolbar->setUndoActionEnabled(enable);
}

void StatesUi::setRedoButtonEnabled(bool enable)
{
    this->toolbar->setRedoActionEnabled(enable);
}
