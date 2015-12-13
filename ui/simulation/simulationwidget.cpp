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
#include "simulationwidget.h"

// Qt classes
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPrinter>
#include <QToolBar>
#include <QVBoxLayout>
#include <QScrollArea>

// StateS classes
#include "machine.h"
#include "signaltimeline.h"
#include "clocktimeline.h"
#include "input.h"
#include "output.h"
#include "simulatortab.h"
#include "machinesimulator.h"
#include "svgimagegenerator.h"


SimulationWidget::SimulationWidget(shared_ptr<Machine> machine, QWidget* parent) :
    QMainWindow(parent)
{
    this->setWindowIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/StateS"))));
    this->setWindowTitle(tr("StateS timeline visualizer"));

    this->toolBar = this->addToolBar(tr("Tools"));
    this->toolBar->setIconSize(QSize(64, 64));

    QIcon exportPdfIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/export_PDF")));
    QAction* action = new QAction(exportPdfIcon, tr("Export to PDF"), this);
    connect(action, &QAction::triggered, this, &SimulationWidget::exportToPDF);

    this->actionDetach = new QAction(tr("Detach as independant window"), this);
    connect(this->actionDetach, &QAction::triggered, this, &SimulationWidget::setMeFree);

    this->toolBar->addAction(action);
    this->toolBar->addAction(this->actionDetach);


    // Add resources in a scroll area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent");

    scrollArea->setWidget(new QWidget());

    this->setCentralWidget(scrollArea);

    shared_ptr<MachineSimulator> simulator = machine->getSimulator();
    shared_ptr<Clock>            clock     = simulator->getClock();

    connect(simulator.get(), &MachineSimulator::outputDelayChangedEvent, this, &SimulationWidget::delayOutputOptionTriggered); // Relay option change event

    QVBoxLayout* layout = new QVBoxLayout(scrollArea->widget());
    layout->setAlignment(Qt::AlignTop);

    QLabel* titleClock = new QLabel("<b>" + tr("Clock") + "</b>");
    titleClock->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleClock);
    layout->addWidget(new ClockTimeLine(clock));

    if (machine->getInputs().count() != 0)
    {
        QLabel* titleInputs = new QLabel("<b>" + tr("Inputs") + "</b>");
        titleInputs->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleInputs);

        foreach (shared_ptr<Input> var, machine->getInputs())
        {
            SignalTimeline* varTL = new SignalTimeline(3, nullptr, var, clock, true);
            layout->addWidget(varTL);
        }
    }

    if (machine->getOutputs().count() != 0)
    {
        QLabel* titleOutputs = new QLabel("<b>" + tr("Outputs") + "</b>");
        titleOutputs->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleOutputs);

        foreach (shared_ptr<Output> var, machine->getOutputs())
        {
            SignalTimeline* varTL = new SignalTimeline(0, this, var, clock);
            layout->addWidget(varTL);
        }
    }

    if (machine->getLocalVariables().count() != 0)
    {
        QLabel* titleVariables = new QLabel("<b>" + tr("Local variables") + "</b>");
        titleVariables->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleVariables);

        foreach (shared_ptr<Signal> var, machine->getLocalVariables())
        {
            SignalTimeline* varTL = new SignalTimeline(0, this, var, clock);
            layout->addWidget(varTL);
        }
    }
}

void SimulationWidget::closeEvent(QCloseEvent* event)
{
    event->ignore();
    emit detachTimelineEvent(false);
}

void SimulationWidget::mousePressEvent(QMouseEvent* event)
{
    // TODO: handle only left button
    this->separatorPosition = event->x();
    repaint();

    QWidget::mousePressEvent(event);
}

void SimulationWidget::mouseMoveEvent(QMouseEvent* event)
{
    // This event is only called if we have clicked first
    this->separatorPosition = event->x();
    repaint();

    QWidget::mouseMoveEvent(event);

}

void SimulationWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.drawLine(this->separatorPosition, 0, this->separatorPosition, this->height());

}

void SimulationWidget::exportToPDF()
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

void SimulationWidget::setMeFree()
{
    disconnect(this->actionDetach, &QAction::triggered, this, &SimulationWidget::setMeFree);

    emit detachTimelineEvent(true);

    this->actionDetach->setText(tr("Attach to main window"));
    connect(this->actionDetach, &QAction::triggered, this, &SimulationWidget::bindMe);
}

void SimulationWidget::bindMe()
{
    disconnect(this->actionDetach, &QAction::triggered, this, &SimulationWidget::bindMe);

    emit detachTimelineEvent(false);

    this->actionDetach->setText(tr("Detach as independant window"));
    connect(this->actionDetach, &QAction::triggered, this, &SimulationWidget::setMeFree);
}

void SimulationWidget::delayOutputOptionTriggered(bool activated)
{
    if (activated)
        emit outputDelayChangedEvent(1);
    else
        emit outputDelayChangedEvent(0);
}
