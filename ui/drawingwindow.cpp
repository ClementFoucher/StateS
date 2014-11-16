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

#include "drawingwindow.h"
#include "ui_drawingwindow.h"

#include "machine.h"
#include "centralwidget.h"
#include "fsm.h"
#include "scenewidget.h"

//
// Constructors
//

DrawingWindow::DrawingWindow(Machine* machine) :
    QMainWindow(nullptr),
    ui(new Ui::DrawingWindow)
{
    this->ui->setupUi(this);

    this->centralWidget = new CentralWidget();
    this->setCentralWidget(this->centralWidget);

    this->setMaximumSize(QApplication::desktop()->availableGeometry().size());

    this->move(QPoint((QApplication::desktop()->availableGeometry().width()-this->width())/2,
                      (QApplication::desktop()->availableGeometry().height()-this->height())/2
                      )
               );

    this->setMachine(machine);
}

//
// Destructor
//

DrawingWindow::~DrawingWindow()
{
    delete ui;
}

//
// Setters
//

void DrawingWindow::setMachine(Machine* machine)
{
    this->centralWidget->setMachine(machine);

    if (machine != nullptr)
    {
        this->ui->actionExport->setEnabled(true);
        this->ui->actionSave->setEnabled(true);
        this->ui->actionClear->setEnabled(true);
    }
    else
    {
        this->ui->actionExport->setEnabled(false);
        this->ui->actionSave->setEnabled(false);
        this->ui->actionClear->setEnabled(true);
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

void DrawingWindow::on_actionNew_triggered()
{
    newMachine();
}

void DrawingWindow::on_actionClear_triggered()
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

void DrawingWindow::on_actionExport_triggered()
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

void DrawingWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save machine"), QString(), "*.SfsmS");

    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith(".SfsmS", Qt::CaseInsensitive))
            fileName += ".SfsmS";

        machine->saveMachine(fileName);
    }
}

void DrawingWindow::on_actionLoad_triggered()
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
