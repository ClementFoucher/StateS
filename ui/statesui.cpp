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

// StateS classes
#include "states.h"
#include "fsm.h"
#include "scenewidget.h"
#include "resourcebar.h"
#include "genericscene.h"


//
// Constructors
//

StatesUi::StatesUi(Machine* machine) :
    QMainWindow(nullptr)
{
    this->setWindowIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/StateS"))));
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
    this->actionSave->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/save"))));
    this->actionSave->setText(tr("Save"));
    this->actionSave->setToolTip(tr("Save machine"));
    connect(this->actionSave, &QAction::triggered, this, &StatesUi::saveMachineRequestEvent);
    mainToolBar->addAction(this->actionSave);

    this->actionLoad = new QAction(this);
    this->actionLoad->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/load"))));
    this->actionLoad->setText(tr("Load"));
    this->actionLoad->setToolTip(tr("Load machine"));
    connect(this->actionLoad, &QAction::triggered, this, &StatesUi::loadMachineRequestEvent);
    mainToolBar->addAction(this->actionLoad);

    mainToolBar->addSeparator();

    this->actionNewFsm = new QAction(this);
    this->actionNewFsm->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/new_FSM"))));
    this->actionNewFsm->setText(tr("New FSM"));
    this->actionNewFsm->setToolTip(tr("Create new FSM"));
    connect(this->actionNewFsm, &QAction::triggered, this, &StatesUi::newMachineRequestEvent);
    mainToolBar->addAction(this->actionNewFsm);

    this->actionClear = new QAction(this);
    this->actionClear->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/clear"))));
    this->actionClear->setText(tr("Clear"));
    this->actionClear->setToolTip(tr("Clear machine"));
    connect(this->actionClear, &QAction::triggered, this, &StatesUi::clearMachineRequestEvent);
    mainToolBar->addAction(this->actionClear);

    mainToolBar->addSeparator();

    this->actionExportPdf = new QAction(this);
    this->actionExportPdf->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/export_PDF"))));
    this->actionExportPdf->setText(tr("Export"));
    this->actionExportPdf->setToolTip(tr("Export machine to PDF file"));
    connect(this->actionExportPdf, &QAction::triggered, this, &StatesUi::exportPdfRequestEvent);
    mainToolBar->addAction(this->actionExportPdf);


    // Add scene and resource bar
    QSplitter* splitter = new QSplitter();
    splitter->setOrientation(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    this->setCentralWidget(splitter);

    this->machineDisplayArea = new SceneWidget(splitter);
    this->resourcesBar = new ResourceBar(splitter);

    QList<int> length;
    length.append(splitter->sizeHint().width()-50);
    length.append(50);
    splitter->setSizes(length);

    connect(this->resourcesBar, &ResourceBar::simulationToggled, this, &StatesUi::handleSimulationToggled);

    this->setMachine(machine);
}

//
// Setters
//

void StatesUi::setMachine(Machine* machine)
{
//    this->centralWidget->setMachine(machine);
    this->resourcesBar->setMachine(machine);
    this->machineDisplayArea->setMachine(machine, resourcesBar);

    if (machine != nullptr)
    {
        this->actionExportPdf->setEnabled(true);
        this->actionSave->setEnabled(true);
        this->actionClear->setEnabled(true);
    }
    else
    {
        this->actionExportPdf->setEnabled(false);
        this->actionSave->setEnabled(false);
        this->actionClear->setEnabled(true);
    }

    this->machine = machine;
}

void StatesUi::clearMachine()
{
    setMachine(nullptr);

    delete machine;
    machine = nullptr;
}

void StatesUi::newMachine()
{
    if (this->machine != nullptr)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Clear current machine?") + "<br />" + tr("Unsaved changes will be lost"), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            clearMachine();
            setMachine(new Fsm());
        }
    }
    else
    {
        setMachine(new Fsm());
    }
}

void StatesUi::newMachineRequestEvent()
{
    newMachine();
}

void StatesUi::clearMachineRequestEvent()
{
    if (this->machine != nullptr)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete current machine?") + "<br />" + tr("Unsaved changes will be lost"), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
            clearMachine();
    }
}

void StatesUi::closeEvent(QCloseEvent *)
{
    // When main window closing, we should terminate application
    // This is not done automatically if another window is open

    // Setting machine to false should remove child widgets
    // and close such windows
    setMachine(nullptr);

}

void StatesUi::exportPdfRequestEvent()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export machine to PDF"), QString(), "*.pdf");

    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
            fileName += ".pdf";

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPrinter::A4);
        printer.setPageOrientation(QPageLayout::Landscape);

        QPainter painter(&printer);

        SceneWidget* sceneView = this->machineDisplayArea;
        sceneView->scene()->clearSelection();
        sceneView->scene()->render(&painter);
    }
}

void StatesUi::saveMachineRequestEvent()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save machine"), QString(), "*.SfsmS");

    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".SfsmS", Qt::CaseInsensitive))
            fileName += ".SfsmS";

        machine->saveMachine(fileName);
    }
}

void StatesUi::loadMachineRequestEvent()
{
    if (this->machine != nullptr)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Overwrite current machine?") + "<br />" + tr("Unsaved changes will be lost"), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Load machine"), QString(), "*.SfsmS");
            if (fileName.count() != 0)
            {
                clearMachine();
                setMachine(new Fsm(fileName));
            }

        }
    }
    else
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Load machine"), QString(), "*.SfsmS");

        if (fileName.count() != 0)
        {
            machine = new Fsm(fileName);
            setMachine(machine);
        }
    }


}

void StatesUi::handleSimulationToggled()
{
    ((GenericScene*)(machineDisplayArea->scene()))->simulationModeChanged();
}

