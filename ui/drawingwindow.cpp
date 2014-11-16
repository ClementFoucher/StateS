/*
 * Copyright © 2014 Clément Foucher
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

#include <QDebug>
#include <QFileDialog>
#include <QPrinter>
#include <QTranslator>
#include <QScreen>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QIcon>
#include <QToolBar>
#include <QApplication>

#include "drawingwindow.h"

#include "states.h"
#include "machine.h"
#include "centralwidget.h"
#include "fsm.h"
#include "scenewidget.h"

//
// Constructors
//

DrawingWindow::DrawingWindow(Machine* machine) :
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
    mainToolBar->setAllowedAreas(Qt::AllToolBarAreas);
    mainToolBar->setIconSize(QSize(64, 64));
    this->addToolBar(Qt::LeftToolBarArea, mainToolBar);

    this->actionSave = new QAction(this);
    this->actionSave->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/save"))));
    this->actionSave->setText(tr("Save"));
    this->actionSave->setToolTip(tr("Save machine"));
    connect(this->actionSave, SIGNAL(triggered()), this, SLOT(saveMachineRequestEvent()));
    mainToolBar->addAction(this->actionSave);

    this->actionLoad = new QAction(this);
    this->actionLoad->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/load"))));
    this->actionLoad->setText(tr("Load"));
    this->actionLoad->setToolTip(tr("Load machine"));
    connect(this->actionLoad, SIGNAL(triggered()), this, SLOT(loadMachineRequestEvent()));
    mainToolBar->addAction(this->actionLoad);

    mainToolBar->addSeparator();

    this->actionNewFsm = new QAction(this);
    this->actionNewFsm->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/new_FSM"))));
    this->actionNewFsm->setText(tr("New FSM"));
    this->actionNewFsm->setToolTip(tr("Create new FSM"));
    connect(this->actionNewFsm, SIGNAL(triggered()), this, SLOT(newMachineRequestEvent()));
    mainToolBar->addAction(this->actionNewFsm);

    this->actionClear = new QAction(this);
    this->actionClear->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/clear"))));
    this->actionClear->setText(tr("Clear"));
    this->actionClear->setToolTip(tr("Clear machine"));
    connect(this->actionClear, SIGNAL(triggered()), this, SLOT(clearMachineRequestEvent()));
    mainToolBar->addAction(this->actionClear);

    mainToolBar->addSeparator();

    this->actionExportPdf = new QAction(this);
    this->actionExportPdf->setIcon(QIcon(StateS::getPixmapFromSvg(QString(":/icons/export_PDF"))));
    this->actionExportPdf->setText(tr("Export"));
    this->actionExportPdf->setToolTip(tr("Export machine to PDF file"));
    connect(this->actionExportPdf, SIGNAL(triggered()), this, SLOT(exportPdfRequestEvent()));
    mainToolBar->addAction(this->actionExportPdf);

    this->centralWidget = new CentralWidget();
    this->setCentralWidget(this->centralWidget);

    this->setMachine(machine);
}

//
// Setters
//

void DrawingWindow::setMachine(Machine* machine)
{
    this->centralWidget->setMachine(machine);

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

void DrawingWindow::clearMachine()
{
    setMachine(nullptr);

    delete machine;
    machine = nullptr;
}

void DrawingWindow::newMachine()
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

void DrawingWindow::newMachineRequestEvent()
{
    newMachine();
}

void DrawingWindow::clearMachineRequestEvent()
{
    if (this->machine != nullptr)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete current machine?") + "<br />" + tr("Unsaved changes will be lost"), QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::StandardButton::Ok)
            clearMachine();
    }
}

void DrawingWindow::closeEvent(QCloseEvent *)
{
    // When main window closing, we should terminate application
    // This is not done automatically if another window is open

    // Setting machine to false should remove child widgets
    // and close such windows
    setMachine(nullptr);

}

void DrawingWindow::exportPdfRequestEvent()
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

        SceneWidget* sceneView = centralWidget->getMachineDisplayArea();
        sceneView->scene()->clearSelection();
        sceneView->scene()->render(&painter);
    }
}

void DrawingWindow::saveMachineRequestEvent()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save machine"), QString(), "*.SfsmS");

    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".SfsmS", Qt::CaseInsensitive))
            fileName += ".SfsmS";

        machine->saveMachine(fileName);
    }
}

void DrawingWindow::loadMachineRequestEvent()
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
