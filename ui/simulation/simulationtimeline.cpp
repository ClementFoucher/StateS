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

#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QList>
#include <QMouseEvent>
#include <QPainter>
#include <QPrinter>

#include "simulationtimeline.h"

#include "logicvariable.h"
#include "machine.h"
#include "variabletimeline.h"
#include "io.h"
#include "clocktimeline.h"
#include "input.h"
#include "output.h"

SimulationTimeLine::SimulationTimeLine(Machine* machine, Clock* clock, QWidget* parent) :
    QMainWindow(parent)
{

    this->machine = machine;

    this->toolBar = this->addToolBar(tr("Tools"));
    this->toolBar->setIconSize(QSize(64, 64));
    QIcon exportPdfIcon;
    exportPdfIcon.addFile(QStringLiteral(":/icons/export_pdf_icon"));
    QAction* action = new QAction(exportPdfIcon, tr("Export to PDF"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(exportToPDF()));

    this->toolBar->addAction(action);


    this->setCentralWidget(new QWidget());
    this->layout = new QVBoxLayout(this->centralWidget());

    QLabel* titleClock = new QLabel("<b>" + tr("Clock") + "</b>");
    titleClock->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleClock);
    layout->addWidget(new ClockTimeLine(clock));

    if (machine->getInputs().count() != 0)
    {
        QLabel* titleInputs = new QLabel("<b>" + tr("Inputs") + "</b>");
        titleInputs->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleInputs);

        foreach (Input* var, machine->getInputs())
        {
            VariableTimeline* varTL = new VariableTimeline(var, clock, true);
            layout->addWidget(varTL);
        }
    }

    if (machine->getOutputs().count() != 0)
    {
        QLabel* titleOutputs = new QLabel("<b>" + tr("Outputs") + "</b>");
        titleOutputs->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleOutputs);

        foreach (Output* var, machine->getOutputs())
        {
            VariableTimeline* varTL = new VariableTimeline(var, clock);
            layout->addWidget(varTL);
        }
    }

    if (machine->getLocalVariables().count() != 0)
    {
        QLabel* titleVariables = new QLabel("<b>" + tr("Local variables") + "</b>");
        titleVariables->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleVariables);

        foreach (LogicVariable* var, machine->getLocalVariables())
        {
            VariableTimeline* varTL = new VariableTimeline(var, clock);
            layout->addWidget(varTL);
        }
    }
}

void SimulationTimeLine::mousePressEvent(QMouseEvent* event)
{
    separatorPosition = event->x();

    QWidget::mousePressEvent(event);

    repaint();
}

void SimulationTimeLine::mouseMoveEvent(QMouseEvent* event)
{
    separatorPosition = event->x();

    QWidget::mouseMoveEvent(event);

    repaint();
}

void SimulationTimeLine::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.drawLine(separatorPosition, 0, separatorPosition, this->height());

}

void SimulationTimeLine::exportToPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export time line to PDF"), QString(), "*.pdf");

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

        // Thanks to Qt doc for this code:
        painter.begin(&printer);
        double xscale = printer.pageRect().width()/double(centralWidget()->width());
        double yscale = printer.pageRect().height()/double(centralWidget()->height());
        double scale = qMin(xscale, yscale);
        painter.translate(printer.paperRect().x() + printer.pageRect().width()/2,
                          printer.paperRect().y() + printer.pageRect().height()/2);
        painter.scale(scale, scale);
        painter.translate(-width()/2, -height()/2);

        centralWidget()->render(&painter, QPoint(), QRegion(), RenderFlag::DrawChildren);
    }
}
