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

// StateS classes
#include "states.h"
#include "resourcebar.h"
#include "vhdlexportdialog.h"
#include "imageexportdialog.h"
#include "displayarea.h"
#include "svgimagegenerator.h"
#include "genericscene.h"


StatesUi::StatesUi() :
    QMainWindow(nullptr)
{
    this->setWindowIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/StateS"))));
    this->updateTitle();
    this->setMaximumSize(QApplication::desktop()->availableGeometry().size());
    this->resize(QApplication::desktop()->availableGeometry().size() - QSize(200, 200));

    // Center window
    this->move(QPoint((QApplication::desktop()->availableGeometry().width()-this->width())/2,
                      (QApplication::desktop()->availableGeometry().height()-this->height())/2
                      )
               );

    // Generate tool bar
    QToolBar* mainToolBar = new QToolBar(this);
    mainToolBar->setMovable(true);
    mainToolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::TopToolBarArea | Qt::BottomToolBarArea);
    mainToolBar->setIconSize(QSize(64, 64));
    this->addToolBar(Qt::LeftToolBarArea, mainToolBar);

    this->actionSave = new QAction(this);
    this->actionSave->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/save_as"))));
    this->actionSave->setText(tr("Save"));
    this->actionSave->setToolTip(tr("Save machine in a new file"));
    connect(this->actionSave, &QAction::triggered, this, &StatesUi::beginSaveAsProcedure);
    mainToolBar->addAction(this->actionSave);

    this->actionSaveCurrent = new QAction(this);
    this->actionSaveCurrent->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/save"))));
    this->actionSaveCurrent->setText(tr("Save as"));
    this->actionSaveCurrent->setToolTip(tr("Update saved file with current content"));
    this->actionSaveCurrent->setEnabled(false);
    connect(this->actionSaveCurrent, &QAction::triggered, this, &StatesUi::beginSaveProcedure);
    mainToolBar->addAction(this->actionSaveCurrent);

    this->actionLoad = new QAction(this);
    this->actionLoad->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/load"))));
    this->actionLoad->setText(tr("Load"));
    this->actionLoad->setToolTip(tr("Load machine from file"));
    connect(this->actionLoad, &QAction::triggered, this, &StatesUi::beginLoadProcedure);
    mainToolBar->addAction(this->actionLoad);

    mainToolBar->addSeparator();

    this->actionNewFsm = new QAction(this);
    //this->actionNewFsm->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/new_FSM"))));
    this->actionNewFsm->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/clear"))));
    this->actionNewFsm->setText(tr("New FSM"));
    this->actionNewFsm->setToolTip(tr("Create new FSM"));
    connect(this->actionNewFsm, &QAction::triggered, this, &StatesUi::beginNewMachineProcedure);
    mainToolBar->addAction(this->actionNewFsm);

   /* this->actionClear = new QAction(this);
    this->actionClear->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/clear"))));
    this->actionClear->setText(tr("Clear"));
    this->actionClear->setToolTip(tr("Clear machine"));
    connect(this->actionClear, &QAction::triggered, this, &StatesUi::beginClearMachineProcedure);
    mainToolBar->addAction(this->actionClear);*/

    mainToolBar->addSeparator();

    this->actionExportImage = new QAction(this);
    this->actionExportImage->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/export_image"))));
    this->actionExportImage->setText(tr("Export to image file"));
    this->actionExportImage->setToolTip(tr("Export machine to an image file"));
    connect(this->actionExportImage, &QAction::triggered, this, &StatesUi::beginExportImageProcedure);
    mainToolBar->addAction(this->actionExportImage);

    this->actionExportVhdl = new QAction(this);
    this->actionExportVhdl->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/export_VHDL"))));
    this->actionExportVhdl->setText(tr("Export to VHDL"));
    this->actionExportVhdl->setToolTip(tr("Export machine to VHDL"));
    connect(this->actionExportVhdl, &QAction::triggered, this, &StatesUi::beginExportVhdlProcedure);
    mainToolBar->addAction(this->actionExportVhdl);

    // Add scene and resource bar
    QSplitter* splitter = new QSplitter();
    splitter->setOrientation(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    this->setCentralWidget(splitter);

    this->displayArea  = new DisplayArea(splitter);
    this->resourceBar = new ResourceBar(splitter);

    connect(this->displayArea, &DisplayArea::itemSelectedEvent,       this, &StatesUi::itemSelectedInSceneEventHandler);
    connect(this->displayArea, &DisplayArea::editSelectedItemEvent,   this, &StatesUi::editSelectedItem);
    connect(this->displayArea, &DisplayArea::renameSelectedItemEvent, this, &StatesUi::renameSelectedItem);

    QList<int> length;
    // Begin with 2/3 - 1/3
    length.append( ( 66 * splitter->sizeHint().width() ) / 100 );
    length.append( ( 33 * splitter->sizeHint().width() ) / 100 );
    splitter->setSizes(length);
}

void StatesUi::setMachine(shared_ptr<Machine> newMachine, const QString& path)
{
    if (! this->machine.expired())
    {
        disconnect(this->machine.lock().get(), &Machine::machineUnsavedStateChanged, this, &StatesUi::machineUnsavedStateChangedEventHandler);
    }

    if (newMachine != nullptr)
    {
        this->actionSave       ->setEnabled(true);
       // this->actionClear      ->setEnabled(true);
        this->actionExportImage->setEnabled(true);
        this->actionExportVhdl ->setEnabled(true);

        if (path.length() != 0)
            //this->actionSaveCurrent->setEnabled(newMachine->isUnsaved());
            this->actionSaveCurrent->setEnabled(true);
        else
            this->actionSaveCurrent->setEnabled(false);

        connect(newMachine.get(), &Machine::machineUnsavedStateChanged, this, &StatesUi::machineUnsavedStateChangedEventHandler);
    }
    else
    {
        this->actionSave       ->setEnabled(false);
        //this->actionClear      ->setEnabled(false);
        this->actionExportImage->setEnabled(false);
        this->actionExportVhdl ->setEnabled(false);
        this->actionSaveCurrent->setEnabled(false);
    }

    this->machine = newMachine;

    this->resourceBar->setMachine(newMachine);
    this->displayArea ->setMachine(newMachine);

    this->setCurrentFilePath(path);
}

void StatesUi::setCurrentFilePath(const QString& path)
{
//    shared_ptr<Machine> l_machine = this->machine.lock();

//    if (l_machine != nullptr)
    {
        this->currentFilePath = path;

        if (path.length() != 0)
//            this->actionSaveCurrent->setEnabled(l_machine->isUnsaved());
            this->actionSaveCurrent->setEnabled(true);
        else
            this->actionSaveCurrent->setEnabled(false);
    }

    this->updateTitle();
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
        unique_ptr<ImageExportDialog> exportOptions(new ImageExportDialog(l_machine->getName(), this->getCurrentDirPath()));
        exportOptions->setModal(true);

        exportOptions->exec();

        if (exportOptions->result() == QDialog::Accepted)
        {
            QString filePath = exportOptions->getFilePath();

            // TODO: Should not act directly on scene
            this->displayArea->getScene()->clearSelection();

            QString comment = tr("Created with") + " StateS v." + StateS::getVersion();

            if (exportOptions->includeComponent() == true)
            {
                MachineImageExporter::exportMachineAsImage(filePath, l_machine->getName(), comment, exportOptions->getImageFormat(), this->displayArea->getScene(), resourceBar->getComponentVisualizationScene().get());
            }
            else
            {
                MachineImageExporter::exportMachineAsImage(filePath, l_machine->getName(), comment, exportOptions->getImageFormat(), this->displayArea->getScene());
            }

        }
    }
}

void StatesUi::beginExportVhdlProcedure()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        unique_ptr<VhdlExportDialog> exportOptions(new VhdlExportDialog(l_machine->getName(), this->getCurrentDirPath()));
        exportOptions->setModal(true);

        exportOptions->exec();

        if (exportOptions->result() == QDialog::Accepted)
        {
            QString filePath = exportOptions->getFilePath();

            l_machine->exportAsVhdl(filePath, exportOptions->isResetPositive(), exportOptions->prefixIOs());
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
  /*  if (isUnsaved)
        this->actionSaveCurrent->setEnabled(true);
    else
        this->actionSaveCurrent->setEnabled(false);*/

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

            emit this->saveMachineRequestEvent(fileName);
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
        emit saveMachineInCurrentFileRequestEvent();
}

void StatesUi::updateTitle()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine == nullptr)
    {
        this->setWindowTitle("StateS");
    }
    else if (this->currentFilePath != QString::null)
    {
        QString title = "StateS — " + this->currentFilePath;
        if (l_machine->isUnsaved())
            title += "*";

        this->setWindowTitle(title);
    }
    else
    {
        this->setWindowTitle("StateS — (" + tr("Unsaved machine") + ")");
    }
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
        userConfirmed = true;

    return userConfirmed;
}

QString StatesUi::getCurrentDirPath() const
{
    return this->currentFilePath;
}
