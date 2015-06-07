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
#include <QPrinter>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QToolBar>
#include <QApplication>
#include <QSplitter>
#include <QAction>
#include <QStackedWidget>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QSvgGenerator>
#include <QPixmap>

// StateS classes
#include "states.h"
#include "fsm.h"
#include "scenewidget.h"
#include "resourcebar.h"
#include "genericscene.h"
#include "fsmvhdlexport.h"
#include "vhdlexportoptions.h"
#include "simulationwidget.h"
#include "imageexportoptions.h"


//
// Constructors
//

StatesUi::StatesUi(shared_ptr<Machine> machine) :
    QMainWindow(nullptr)
{
    this->setWindowIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/StateS"))));
    if (machine != nullptr)
        this->setWindowTitle("StateS — (" + tr("Unsaved machine") + ")");
    else
        this->setWindowTitle("StateS");
    this->setMaximumSize(QApplication::desktop()->availableGeometry().size());
    this->resize(QApplication::desktop()->availableGeometry().size() - QSize(200, 200));

    // Center window
    this->move(QPoint((QApplication::desktop()->availableGeometry().width()-this->width())/2,
                      (QApplication::desktop()->availableGeometry().height()-this->height())/2
                      )
               );

    QToolBar* mainToolBar = new QToolBar(this);
    mainToolBar->setMovable(true);
    mainToolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::TopToolBarArea | Qt::BottomToolBarArea);
    mainToolBar->setIconSize(QSize(64, 64));
    this->addToolBar(Qt::LeftToolBarArea, mainToolBar);

    this->actionSave = new QAction(this);
    this->actionSave->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/save_as"))));
    this->actionSave->setText(tr("Save"));
    this->actionSave->setToolTip(tr("Save machine in a new file"));
    connect(this->actionSave, &QAction::triggered, this, &StatesUi::saveMachineRequestEventHandler);
    mainToolBar->addAction(this->actionSave);

    this->actionSaveCurrent = new QAction(this);
    this->actionSaveCurrent->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/save"))));
    this->actionSaveCurrent->setText(tr("Save as"));
    this->actionSaveCurrent->setToolTip(tr("Update saved file with current content"));
    this->actionSaveCurrent->setEnabled(false);
    connect(this->actionSaveCurrent, &QAction::triggered, this, &StatesUi::saveMachineOnCurrentFileEventHandler);
    mainToolBar->addAction(this->actionSaveCurrent);

    this->actionLoad = new QAction(this);
    this->actionLoad->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/load"))));
    this->actionLoad->setText(tr("Load"));
    this->actionLoad->setToolTip(tr("Load machine from file"));
    connect(this->actionLoad, &QAction::triggered, this, &StatesUi::loadMachineRequestEventHandler);
    mainToolBar->addAction(this->actionLoad);

    mainToolBar->addSeparator();

    /*this->actionNewFsm = new QAction(this);
    this->actionNewFsm->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/new_FSM"))));
    this->actionNewFsm->setText(tr("New FSM"));
    this->actionNewFsm->setToolTip(tr("Create new FSM"));
    connect(this->actionNewFsm, &QAction::triggered, this, &StatesUi::newMachineRequestEvent);
    mainToolBar->addAction(this->actionNewFsm);*/

    this->actionClear = new QAction(this);
    this->actionClear->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/clear"))));
    this->actionClear->setText(tr("Clear"));
    this->actionClear->setToolTip(tr("Clear machine"));
    connect(this->actionClear, &QAction::triggered, this, &StatesUi::newMachineRequestEventHandler);
    //connect(this->actionClear, &QAction::triggered, this, &StatesUi::clearMachineRequestEvent);
    mainToolBar->addAction(this->actionClear);

    mainToolBar->addSeparator();

    this->actionExportImage = new QAction(this);
    this->actionExportImage->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/export_image"))));
    this->actionExportImage->setText(tr("Export to image file"));
    this->actionExportImage->setToolTip(tr("Export machine to an image file"));
    connect(this->actionExportImage, &QAction::triggered, this, &StatesUi::exportImageRequestEventHandler);
    mainToolBar->addAction(this->actionExportImage);

    this->actionExportVhdl = new QAction(this);
    this->actionExportVhdl->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/export_VHDL"))));
    this->actionExportVhdl->setText(tr("Export to VHDL"));
    this->actionExportVhdl->setToolTip(tr("Export machine to VHDL"));
    connect(this->actionExportVhdl, &QAction::triggered, this, &StatesUi::exportVhdlRequestEventHandler);
    mainToolBar->addAction(this->actionExportVhdl);

    // Add scene and resource bar
    QSplitter* splitter = new QSplitter();
    splitter->setOrientation(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    this->setCentralWidget(splitter);

    this->mainDisplayArea = new QStackedWidget(splitter);
    this->machineDisplayArea = new SceneWidget();
    this->mainDisplayArea->addWidget(this->machineDisplayArea);
    this->resourcesBar = new ResourceBar(nullptr, splitter);

    QList<int> length;
    length.append(splitter->sizeHint().width()-50);
    length.append(50);
    splitter->setSizes(length);

    connect(this->resourcesBar, &ResourceBar::simulationToggledEvent,  this, &StatesUi::simulationToggledEventHandler);
    connect(this->resourcesBar, &ResourceBar::triggerViewRequestEvent, this, &StatesUi::triggerViewEventHandler);

    this->setMachine(machine);
}

void StatesUi::setMachine(shared_ptr<Machine> machine)
{
    shared_ptr<Machine> oldMachine = this->machine.lock();

    if (oldMachine != nullptr)
        disconnect(oldMachine.get(), &Machine::machineLoadedEvent, this, &StatesUi::machineLoadedEventHandler);

    if (machine != nullptr)
        connect(machine.get(), &Machine::machineLoadedEvent, this, &StatesUi::machineLoadedEventHandler);

    this->resourcesBar->setMachine(machine);
    this->machineDisplayArea->setMachine(machine, resourcesBar);

    if (machine != nullptr)
    {
        this->actionExportImage->setEnabled(true);
        this->actionExportVhdl->setEnabled(true);
        this->actionSave->setEnabled(true);
        this->actionClear->setEnabled(true);
    }
    else
    {
        this->actionExportImage->setEnabled(false);
        this->actionExportVhdl->setEnabled(false);
        this->actionSave->setEnabled(false);
        this->actionClear->setEnabled(true);
    }

    this->machine = machine;
}

void StatesUi::newMachineRequestEventHandler()
{
    bool doNew = false;

    if (this->machine.lock() != nullptr)
    {
        if (this->machine.lock()->isEmpty())
        {
            doNew = true;
        }
        else
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Clear current machine?") + "<br />" + tr("Unsaved changes will be lost."), QMessageBox::Ok | QMessageBox::Cancel);

            if (reply == QMessageBox::StandardButton::Ok)
            {
                doNew = true;
            }
        }
    }
    else
    {
        doNew = true;
    }

    if (doNew)
    {
        this->setWindowTitle("StateS — (" + tr("Unsaved machine") + ")");
        this->actionSaveCurrent->setEnabled(false);
        emit newFsmRequestEvent();
    }
}

void StatesUi::clearMachineRequestEventHandler()
{
    bool doClear = false;

    if (this->machine.lock() != nullptr)
    {
        if (this->machine.lock()->isEmpty())
        {
            doClear = true;
        }
        else
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete current machine?") + "<br />" + tr("Unsaved changes will be lost."), QMessageBox::Ok | QMessageBox::Cancel);

            if (reply == QMessageBox::StandardButton::Ok)
            {
                doClear = true;
            }
        }

        if (doClear)
        {
            emit clearMachineRequestEvent();
            this->setWindowTitle("StateS — (" + tr("Unsaved machine") + ")");
            this->actionSaveCurrent->setEnabled(false);
        }
    }
}

void StatesUi::closeEvent(QCloseEvent*)
{
    // When main window closing, we should terminate application
    // This is not done automatically if another window is open

    // Setting machine to nullptr should remove child widgets
    // and close such windows (not checked)
    setMachine(nullptr);

}

void StatesUi::keyPressEvent(QKeyEvent* event)
{
    if ( ((event->modifiers() | Qt::CTRL) != 0) && (event->key() == Qt::Key_S) )
    {
        if (this->actionSaveCurrent->isEnabled())
        {
            machine.lock()->saveMachine(this->getCurrentFileEvent());
            // Should make button blink for one second.
            // How to without locking UI?
        }
        else
        {
            this->saveMachineRequestEventHandler();
        }
    }
    else
    {
        event->ignore();
    }
}

void StatesUi::exportImageRequestEventHandler()
{
    unique_ptr<ImageExportOptions> exportOptions(new ImageExportOptions());
    exportOptions->setModal(true);

    exportOptions->exec();

    if (exportOptions->result() == QDialog::Accepted)
    {
        ImageExportOptions::imageFormat selectedFormat = exportOptions->getImageFormat();

        QString fileName;
        unique_ptr<QPainter> painter = nullptr;
        unique_ptr<QPrinter> printer = nullptr;
        unique_ptr<QSvgGenerator> generator = nullptr;
        unique_ptr<QPixmap> pixmap = nullptr;

        if (selectedFormat == ImageExportOptions::imageFormat::pdf)
        {
            fileName = QFileDialog::getSaveFileName(this, tr("Export machine to Pdf"), QString(), "*.pdf");

            if (!fileName.isEmpty())
            {
                if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
                    fileName += ".pdf";

                printer = unique_ptr<QPrinter>(new QPrinter(QPrinter::HighResolution));
                printer->setOutputFormat(QPrinter::PdfFormat);
                printer->setOutputFileName(fileName);
                printer->setPageSize(QPrinter::A4);
                printer->setPageOrientation(QPageLayout::Landscape);

                painter = unique_ptr<QPainter>(new QPainter());

                painter->begin(printer.get());


            }
        }
        else if (selectedFormat == ImageExportOptions::imageFormat::svg)
        {
            fileName = QFileDialog::getSaveFileName(this, tr("Export machine to Svg"), QString(), "*.svg");

            if (!fileName.isEmpty())
            {
                if (!fileName.endsWith(".svg", Qt::CaseInsensitive))
                    fileName += ".svg";

                generator = unique_ptr<QSvgGenerator>(new QSvgGenerator());

                generator->setFileName(fileName);
                QRectF size = this->machineDisplayArea->scene()->sceneRect();
                generator->setSize(size.size().toSize());
                generator->setTitle(tr("Machine"));
                generator->setDescription(tr("Created with") + " StateS v." + StateS::getVersion());

                painter = unique_ptr<QPainter>(new QPainter());

                painter->begin(generator.get());

            }
        }
        else if (selectedFormat == ImageExportOptions::imageFormat::png)
        {
            fileName = QFileDialog::getSaveFileName(this, tr("Export machine to Png"), QString(), "*.png");

            if (!fileName.isEmpty())
            {
                if (!fileName.endsWith(".png", Qt::CaseInsensitive))
                    fileName += ".png";

                QRectF size = this->machineDisplayArea->scene()->sceneRect();
                pixmap = unique_ptr<QPixmap>(new QPixmap(size.width(), size.height()));
                pixmap->fill();

                painter = unique_ptr<QPainter>(new QPainter());

                painter->begin(pixmap.get());

                painter->setRenderHint(QPainter::Antialiasing);
            }
        }
        else if (selectedFormat == ImageExportOptions::imageFormat::jpg)
        {
            fileName = QFileDialog::getSaveFileName(this, tr("Export machine to Jpeg"), QString(), "*.jpg");

            if (!fileName.isEmpty())
            {
                if (!fileName.endsWith(".jpg", Qt::CaseInsensitive))
                    fileName += ".jpg";

                QRectF size = this->machineDisplayArea->scene()->sceneRect();
                pixmap = unique_ptr<QPixmap>(new QPixmap(size.width(), size.height()));
                pixmap->fill();

                painter = unique_ptr<QPainter>(new QPainter());

                painter->begin(pixmap.get());

                painter->setRenderHint(QPainter::Antialiasing);
            }
        }



        if (painter != nullptr)
        {
            QGraphicsScene* scene = this->machineDisplayArea->scene();
            scene->clearSelection();
            scene->render(painter.get());

            if (exportOptions->includeComponent() == true)
            {
                QRectF totalSceneSize = scene->sceneRect();

                scene = resourcesBar->getComponentVisualizationScene().get();
                QRectF componentSize;

                if (selectedFormat == ImageExportOptions::imageFormat::pdf)
                {
                    componentSize.setWidth(3000);
                    componentSize.setHeight(2000);
                    componentSize.translate(50,50);
                }
                else
                {
                    // Make component a size of 1/5th the machine
                    componentSize.setWidth(totalSceneSize.width()/5);
                    componentSize.setHeight(totalSceneSize.height()/5);
                    componentSize.translate(50,50);
                }

                scene->render(painter.get(), componentSize);
            }

            painter->end();
        }

        if (pixmap != nullptr)
        {
            pixmap->save(fileName);
        }
    }
}

void StatesUi::exportVhdlRequestEventHandler()
{
    unique_ptr<VhdlExportOptions> exportOptions(new VhdlExportOptions());
    exportOptions->setModal(true);

    exportOptions->exec();

    if (exportOptions->result() == QDialog::Accepted)
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Export machine to VHDL"), QString(), "*.vhdl");

        if (!fileName.isEmpty())
        {
            if (!fileName.endsWith(".vhdl", Qt::CaseInsensitive))
                fileName += ".vhdl";

            FsmVhdlExport::exportFSM(dynamic_pointer_cast<Fsm>(machine.lock()), fileName, exportOptions->isResetPositive(), exportOptions->prefixIOs());
        }
    }
}

void StatesUi::saveMachineRequestEventHandler()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save machine"), QString(), "*.SfsmS");

    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".SfsmS", Qt::CaseInsensitive))
            fileName += ".SfsmS";

        machine.lock()->saveMachine(fileName);
        emit this->machineSavedEvent(fileName);

        this->setWindowTitle("StateS — " + this->getCurrentFileEvent());
        this->actionSaveCurrent->setEnabled(true);
    }
}

void StatesUi::loadMachineRequestEventHandler()
{
    bool doLoad = false;

    if (this->machine.lock() != nullptr)
    {
        if (this->machine.lock()->isEmpty())
            doLoad = true;
        else
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Overwrite current machine?") + "<br />" + tr("Unsaved changes will be lost."), QMessageBox::Ok | QMessageBox::Cancel);

            if (reply == QMessageBox::StandardButton::Ok)
            {
                doLoad = true;
            }
        }
    }
    else
        doLoad = true;

    if (doLoad)
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Load machine"), QString(), "*.SfsmS");

        if (fileName.count() != 0)
        {
            emit loadMachineRequestEvent(fileName);
        }
    }
}

void StatesUi::simulationToggledEventHandler()
{
    ((GenericScene*)(this->machineDisplayArea->scene()))->simulationModeChanged();

    this->timeline = this->resourcesBar->getTimeline();
    if (this->timeline != nullptr)
    {
        this->mainDisplayArea->addWidget(this->timeline);
        connect(this->timeline, &SimulationWidget::detachTimelineEvent, this, &StatesUi::detachTimelineEventHandler);
    }
}

void StatesUi::triggerViewEventHandler()
{
    if (this->mainDisplayArea->count() > 1)
    {
        if (this->mainDisplayArea->currentIndex() == 0)
            this->mainDisplayArea->setCurrentIndex(1);
        else
            this->mainDisplayArea->setCurrentIndex(0);
    }
}

void StatesUi::detachTimelineEventHandler(bool detach)
{
    if (detach)
    {
        this->timeline->setParent(nullptr);
        this->timeline->show();
    }
    else
    {
        this->mainDisplayArea->addWidget(this->timeline);
    }
}

void StatesUi::machineLoadedEventHandler()
{
    // Reset display
    this->machineDisplayArea->setMachine(this->machine.lock(), this->resourcesBar);
    this->setWindowTitle("StateS — " + this->getCurrentFileEvent());
    this->actionSaveCurrent->setEnabled(true);
}

void StatesUi::saveMachineOnCurrentFileEventHandler()
{
    bool doSave = false;

    if (this->machine.lock() != nullptr)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Update content of file") + " " + this->getCurrentFileEvent() + " " + tr("with current machine?"), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            doSave = true;
        }
    }
    else
        doSave = true;

    if (doSave)
    {
        machine.lock()->saveMachine(this->getCurrentFileEvent());
    }
}

