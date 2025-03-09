/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "timelinewidget.h"

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
#include "machinemanager.h"
#include "machine.h"
#include "variabletimeline.h"
#include "clocktimeline.h"
#include "statetimeline.h"
#include "simulatedmachine.h"
#include "pixmapgenerator.h"


TimelineWidget::TimelineWidget(QWidget* parent) :
	QMainWindow(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	/////
	// Configure window
	this->setWindowIcon(QIcon(PixmapGenerator::getStatesWindowIcon()));
	this->setWindowTitle(tr("StateS timeline visualizer"));

	/////
	// Build toolbar
	this->toolBar = this->addToolBar(tr("Tools"));
	this->toolBar->setIconSize(QSize(64, 64));

	QIcon exportPdfIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/export_PDF")));
	QAction* action = new QAction(exportPdfIcon, tr("Export to PDF"), this);
	connect(action, &QAction::triggered, this, &TimelineWidget::exportToPDF);

	QIcon detachWindowIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/detach_window")));
	this->actionDetach = new QAction(detachWindowIcon, tr("Detach as independant window"), this);
	connect(this->actionDetach, &QAction::triggered, this, &TimelineWidget::setMeFree);

	this->toolBar->addAction(action);
	this->toolBar->addAction(this->actionDetach);

	/////
	// Add timelines in a scroll area
	QScrollArea* scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setStyleSheet("background-color: transparent");
	this->setCentralWidget(scrollArea);

	this->displayWidget = new QWidget();
	auto hLayout = new QHBoxLayout(this->displayWidget);
	scrollArea->setWidget(this->displayWidget);

	auto vLayout = new QVBoxLayout();
	vLayout->setAlignment(Qt::AlignTop);
	hLayout->addLayout(vLayout);
	hLayout->addStretch();

	// Clock
	QLabel* titleClock = new QLabel("<b>" + tr("Clock") + "</b>");
	titleClock->setAlignment(Qt::AlignCenter);

	vLayout->addWidget(titleClock);
	vLayout->addWidget(new ClockTimeLine());

	// Inputs
	auto inputIds = machine->getInputVariablesIds();
	if (inputIds.count() != 0)
	{
		QLabel* titleInputs = new QLabel("<b>" + tr("Inputs") + "</b>");
		titleInputs->setAlignment(Qt::AlignCenter);

		vLayout->addWidget(titleInputs);

		for (auto& varId : inputIds)
		{
			VariableTimeline* varTL = new VariableTimeline(3, varId);
			vLayout->addWidget(varTL);
		}
	}

	// Internal variables
	QLabel* titleVariables = new QLabel("<b>" + tr("Internal variables") + "</b>");
	titleVariables->setAlignment(Qt::AlignCenter);
	vLayout->addWidget(titleVariables);

	vLayout->addWidget(new StateTimeLine());

	for (auto& varId : machine->getInternalVariablesIds())
	{
		VariableTimeline* varTL = new VariableTimeline(0, varId);
		vLayout->addWidget(varTL);
	}

	// Outputs
	auto outputsIds = machine->getOutputVariablesIds();
	if (outputsIds.count() != 0)
	{
		QLabel* titleOutputs = new QLabel("<b>" + tr("Outputs") + "</b>");
		titleOutputs->setAlignment(Qt::AlignCenter);

		vLayout->addWidget(titleOutputs);

		for (auto& varId : outputsIds)
		{
			VariableTimeline* varTL = new VariableTimeline(0, varId);
			vLayout->addWidget(varTL);
		}
	}
}

void TimelineWidget::closeEvent(QCloseEvent* event)
{
	event->ignore();
	this->bindMe();
}

void TimelineWidget::mousePressEvent(QMouseEvent* event)
{
	// TODO: handle only left button
	this->separatorPosition = event->pos().x();
	repaint();

	QWidget::mousePressEvent(event);
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
	// This event is only called if we have clicked first
	this->separatorPosition = event->pos().x();
	repaint();

	QWidget::mouseMoveEvent(event);
}

void TimelineWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	painter.drawLine(this->separatorPosition, 0, this->separatorPosition, this->height());
}

void TimelineWidget::exportToPDF()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export time line to PDF"), QString(), "*.pdf");

	if (!fileName.isEmpty())
	{
		if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
		{
			fileName += ".pdf";
		}

		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(fileName);
		printer.setPageSize(QPageSize(QPageSize::A4));
		printer.setPageOrientation(QPageLayout::Landscape);

		QPainter painter(&printer);

		painter.begin(&printer);
		double xscale = printer.pageRect(QPrinter::DevicePixel).width()/((double)(this->displayWidget->width()));
		double yscale = printer.pageRect(QPrinter::DevicePixel).height()/((double)(this->displayWidget->height()));
		double scale = qMin(xscale, yscale);
		painter.scale(scale, scale);
		this->displayWidget->render(&painter, QPoint(), QRegion(), RenderFlag::DrawChildren);
		painter.end();
	}
}

void TimelineWidget::setMeFree()
{
	disconnect(this->actionDetach, &QAction::triggered, this, &TimelineWidget::setMeFree);

	emit detachTimelineEvent(true);

	this->actionDetach->setText(tr("Attach to main window"));

	QIcon attachWindowIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/attach_window")));
	this->actionDetach->setIcon(attachWindowIcon);

	connect(this->actionDetach, &QAction::triggered, this, &TimelineWidget::bindMe);
}

void TimelineWidget::bindMe()
{
	disconnect(this->actionDetach, &QAction::triggered, this, &TimelineWidget::bindMe);

	emit detachTimelineEvent(false);

	this->actionDetach->setText(tr("Detach as independant window"));

	QIcon detachWindowIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/detach_window")));
	this->actionDetach->setIcon(detachWindowIcon);

	connect(this->actionDetach, &QAction::triggered, this, &TimelineWidget::setMeFree);
}
