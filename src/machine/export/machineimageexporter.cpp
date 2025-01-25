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
#include "machineimageexporter.h"

// Qt classes
#include <QPainter>
#include <QPrinter>
#include <QPageSize>
#include <QSvgGenerator>
#include <QGraphicsTextItem>

// StateS classes
#include "machinemanager.h"
#include "genericscene.h"
#include "machine.h"
#include "variable.h"


MachineImageExporter::MachineImageExporter(GenericScene* scene, shared_ptr<QGraphicsScene> component)
{
	this->scene = scene;
	this->component = component;

	this->includeComponent = false;
	this->includeConstant = false;
	this->includeVariables = false;
	this->addBorder = false;
	this->mainSceneRatio = 3;
}

void MachineImageExporter::setDisplayComponent(bool doDisplay)
{
	this->includeComponent = doDisplay;
}

void MachineImageExporter::setDisplayConstants(bool doDisplay)
{
	this->includeConstant = doDisplay;
}

void MachineImageExporter::setDisplayVariables(bool doDisplay)
{
	this->includeVariables = doDisplay;
}

void MachineImageExporter::setDisplayBorder(bool doDisplay)
{
	this->addBorder = doDisplay;
}

void MachineImageExporter::setMainSceneRatio(uint sceneRatio)
{
	this->mainSceneRatio = sceneRatio;
}

void MachineImageExporter::setInfoPos(LeftRight_t pos)
{
	this->infoPosition = pos;
}

shared_ptr<QPixmap> MachineImageExporter::renderPreview(QSizeF previewSize)
{
	shared_ptr<QPixmap> generatedPixmap;

	this->pageRect = QRectF(QPointF(0,0), previewSize);
	this->renderAreaWithoutBordersRect = this->pageRect;
	this->renderAreaWithoutBordersRect.adjust(0, 0, -2*this->spacer, -2* this->spacer);

	this->generatePrintingRects();

	// Add margins
	this->renderAreaWithoutBordersRect.translate(this->spacer, this->spacer);
	this->scenePrintingRect    .translate(this->spacer, this->spacer);
	this->componentPrintingRect.translate(this->spacer, this->spacer);
	this->constantsPrintingRect.translate(this->spacer, this->spacer);
	this->variablesPrintingRect.translate(this->spacer, this->spacer);

	this->renderBitmap();

	generatedPixmap = this->pixmap;
	this->freeRenderingResources();

	return generatedPixmap;
}

void MachineImageExporter::doExport(const QString& path, ImageFormat_t format, const QString& creator)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	if (format == ImageFormat_t::pdf)
	{
		this->preparePdfPrinter(path, machine->getName(), creator);
		this->renderAreaWithoutBordersRect = this->printer->pageRect(QPrinter::DevicePixel);

		this->pageRect = this->renderAreaWithoutBordersRect; // No adjust (margins already added by printer)
	}
	else
	{
		this->renderAreaWithoutBordersRect = this->scene->sceneRect(); // Use scene as the base size and ratio
		this->renderAreaWithoutBordersRect.setTopLeft(QPointF(0,0));

		this->pageRect = this->renderAreaWithoutBordersRect;
		this->pageRect.adjust(0, 0, 2*this->spacer, 2*this->spacer);
	}

	this->generatePrintingRects();

	if (format == ImageFormat_t::pdf)
	{
		this->renderPdf();
	}
	else
	{
		// Add margins
		this->renderAreaWithoutBordersRect.translate(this->spacer, this->spacer);
		this->scenePrintingRect    .translate(this->spacer, this->spacer);
		this->componentPrintingRect.translate(this->spacer, this->spacer);
		this->constantsPrintingRect.translate(this->spacer, this->spacer);
		this->variablesPrintingRect.translate(this->spacer, this->spacer);

		if (format == ImageFormat_t::svg)
		{
			this->renderSvg(path, machine->getName(), creator);
		}
		else
		{
			this->renderBitmap();
			this->pixmap->save(path);
		}
	}

	// Done, clear rendering ressources
	this->freeRenderingResources();
}

/**
 * @brief MachineImageExporter::generatePrintingRects Generates the position
 * or each part to print depending on the "totalPrintedRect", which is the
 * available drawing area (without borders)
 */
void MachineImageExporter::generatePrintingRects()
{
	this->scenePrintingRect = this->renderAreaWithoutBordersRect;

	if ( (this->includeComponent == true) || (this->includeConstant == true) || (this->includeVariables == true) )
	{
		const qreal totalHeight = this->renderAreaWithoutBordersRect.height();
		const qreal totalWidth  = this->renderAreaWithoutBordersRect.width();
		const qreal infoWidth   = (  totalWidth                       / (this->mainSceneRatio+1) ) - this->spacer/2;
		const qreal sceneWidth  = ( (totalWidth*this->mainSceneRatio) / (this->mainSceneRatio+1) ) - this->spacer/2;

		this->scenePrintingRect.setWidth(sceneWidth);

		// Temporary size
		this->componentPrintingRect = QRectF(0, 0, infoWidth, totalHeight);
		this->constantsPrintingRect = QRectF(0, 0, infoWidth, totalHeight);
		this->variablesPrintingRect = QRectF(0, 0, infoWidth, totalHeight);

		// Horizontal alignment
		if (this->infoPosition == LeftRight_t::left)
		{
			this->scenePrintingRect.translate(infoWidth + this->spacer, 0);
		}
		else
		{
			this->componentPrintingRect.translate(sceneWidth + this->spacer, 0);
			this->constantsPrintingRect.translate(sceneWidth + this->spacer, 0);
			this->variablesPrintingRect.translate(sceneWidth + this->spacer, 0);
		}

		// Vertical alignment
		if (this->includeComponent == true)
		{
			this->constantsPrintingRect.translate(0, totalHeight/2 + this->spacer);
			this->variablesPrintingRect.translate(0, totalHeight/2 + this->spacer);

			if (this->includeConstant == true)
				this->variablesPrintingRect.translate(0, totalHeight/4);
		}
		else
		{
			if (this->includeConstant == true)
				this->variablesPrintingRect.translate(0, totalHeight/2 + this->spacer);
		}

		// Size
		if ( (this->includeComponent == true) && ( (this->includeConstant == true) || (this->includeVariables == true) ) ) // Component and at least a signal display
		{
			this->componentPrintingRect.setHeight(totalHeight / 2);

			if ( (this->includeConstant == true) && (this->includeVariables == true) ) // Both signal displays
			{
				this->constantsPrintingRect.setHeight(totalHeight / 4 - this->spacer);
				this->variablesPrintingRect.setHeight(totalHeight / 4 - this->spacer);
			}
			else // Only one signal display
			{
				this->constantsPrintingRect.setHeight(totalHeight / 2 - this->spacer);
				this->variablesPrintingRect.setHeight(totalHeight / 2 - this->spacer);
			}
		}
		else if (this->includeComponent == true) // Only signal displays
		{
			if ( (this->includeConstant == true) && (this->includeVariables == true) ) // Both signal displays
			{
				this->constantsPrintingRect.setHeight(totalHeight / 2);
				this->variablesPrintingRect.setHeight(totalHeight / 2 - this->spacer);
			}
		}
	}

	this->painter = shared_ptr<QPainter>(new QPainter());
}


void MachineImageExporter::preparePdfPrinter(const QString& path, const QString& title, const QString& creator)
{
	this->printer = shared_ptr<QPrinter>(new QPrinter(QPrinter::HighResolution));

	this->printer->setOutputFormat(QPrinter::PdfFormat);
	this->printer->setPageSize(QPageSize(QPageSize::A4));
	this->printer->setPageOrientation(QPageLayout::Landscape);

	this->printer->setOutputFileName(path);
	this->printer->setCreator(creator);
	this->printer->setDocName(title);
}

void MachineImageExporter::renderPdf()
{
	this->strictBorders = true;

	this->painter->begin(this->printer.get());
	this->renderOnPainter();
	this->painter->end();
}

void MachineImageExporter::renderSvg(const QString& path, const QString& title, const QString& creator)
{
	this->generator = shared_ptr<QSvgGenerator>(new QSvgGenerator());

	this->generator->setSize(this->pageRect.size().toSize());

	this->generator->setFileName(path);
	this->generator->setDescription(creator);
	this->generator->setTitle(title);

	this->strictBorders = false;

	this->painter->begin(this->generator.get());
	this->renderOnPainter();
	this->painter->end();
}

void MachineImageExporter::renderBitmap()
{
	this->pixmap = shared_ptr<QPixmap>(new QPixmap(this->pageRect.size().toSize()));

	this->pixmap->fill();

	this->strictBorders = false;

	this->painter->begin(this->pixmap.get());
	this->painter->setRenderHint(QPainter::Antialiasing);
	this->renderOnPainter();
	this->painter->end();
}

void MachineImageExporter::renderOnPainter()
{
	if (this->painter != nullptr)
	{
		if (this->addBorder == true)
			this->border = shared_ptr<QGraphicsScene>(new QGraphicsScene(this->pageRect));

		this->renderScene();
		this->renderComponent();
		this->renderConstants();
		this->renderVariables();

		this->renderBorder();
	}
}

void MachineImageExporter::renderScene()
{
	if (this->addBorder == true)
		this->prepareBorder(this->scenePrintingRect);

	QRectF actualPrintingRect = this->getActualPrintedRect(this->scene->sceneRect(), this->scenePrintingRect);
	this->scene->render(this->painter.get(), actualPrintingRect);
}

void MachineImageExporter::renderComponent()
{
	if (this->includeComponent == true)
	{
		if (this->addBorder == true)
			this->prepareBorder(this->componentPrintingRect);

		shared_ptr<QGraphicsScene> l_component = this->component.lock();
		if (l_component != nullptr)
		{
			QRectF actualPrintingRect = this->getActualPrintedRect(l_component->sceneRect(), this->componentPrintingRect);
			l_component->render(this->painter.get(), actualPrintingRect);
		}
	}
}

void MachineImageExporter::renderConstants()
{
	if (this->includeConstant == false) return;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	if (this->addBorder == true)
	{
		this->prepareBorder(this->constantsPrintingRect);
	}

	shared_ptr<QGraphicsScene> constantScene(new QGraphicsScene());
	QGraphicsTextItem* constantsTitle = new QGraphicsTextItem(tr("Constants:"));
	constantScene->addItem(constantsTitle);

	int pos = 0;
	for(shared_ptr<Variable> constant : machine->getConstants())
	{
		QString constText = constant->getName();
		if (constant->getSize() > 1)
			constText += "[" + QString::number(constant->getSize()-1) + "..0]";
		constText += " = \"" + constant->getInitialValue().toString() + "\"";

		QGraphicsTextItem* constantItem = new QGraphicsTextItem(constText);
		constantItem->setPos(0, 60 + pos*20);
		constantScene->addItem(constantItem);
		pos++;
	}

	QRectF actualPrintingRect = this->getActualPrintedRect(constantScene->sceneRect(), this->constantsPrintingRect);
	constantScene->render(this->painter.get(), actualPrintingRect);
}

void MachineImageExporter::renderVariables()
{
	if (this->includeVariables == false) return;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	if (this->addBorder == true)
	{
		this->prepareBorder(this->variablesPrintingRect);
	}

	shared_ptr<QGraphicsScene> variableScene(new QGraphicsScene());
	QGraphicsTextItem* text = new QGraphicsTextItem(tr("Variables:"));
	variableScene->addItem(text);

	int pos = 0;
	for(shared_ptr<Variable> variable : machine->getLocalVariables())
	{
		QString varText = variable->getName();
		if (variable->getSize() > 1)
			varText += "[" + QString::number(variable->getSize()-1) + "..0]";
		varText += " ( = \"" + variable->getInitialValue().toString() + "\" @ t=0 )";

		QGraphicsTextItem* variableItem = new QGraphicsTextItem(varText);
		variableItem->setPos(0, 40 + pos*20);
		variableScene->addItem(variableItem);

		pos++;
	}

	QRectF actualPrintingRect = this->getActualPrintedRect(variableScene->sceneRect(), this->variablesPrintingRect);
	variableScene->render(this->painter.get(), actualPrintingRect);
}

void MachineImageExporter::prepareBorder(const QRectF& availablePrintingRect)
{
	QRectF borderRect = availablePrintingRect;
	borderRect.adjust(-this->spacer/2, -this->spacer/2, this->spacer/2, this->spacer/2);

	if (this->strictBorders == true)
	{
		if (borderRect.left() < this->renderAreaWithoutBordersRect.left())
			borderRect.setLeft(this->renderAreaWithoutBordersRect.left());

		if (borderRect.top() < this->renderAreaWithoutBordersRect.top())
			borderRect.setTop(this->renderAreaWithoutBordersRect.top());

		if (borderRect.right() < this->renderAreaWithoutBordersRect.right())
			borderRect.setRight(this->renderAreaWithoutBordersRect.right());

		if (borderRect.bottom() < this->renderAreaWithoutBordersRect.bottom())
			borderRect.setBottom(this->renderAreaWithoutBordersRect.bottom());
	}
	this->border->addRect(borderRect);
}

void MachineImageExporter::renderBorder()
{
	if (this->addBorder == true)
	{
		this->border->render(this->painter.get(), pageRect);
	}
	this->border.reset();
}

QRectF MachineImageExporter::getActualPrintedRect(const QRectF& elementPrintingRect, const QRectF& availablePrintingRect)
{
	QSizeF actualPrintingSize = elementPrintingRect.size();
	actualPrintingSize.scale(availablePrintingRect.size(), Qt::KeepAspectRatio);

	QRectF actualPrintingRect(QPointF(0,0), actualPrintingSize);
	actualPrintingRect.translate(availablePrintingRect.topLeft());

	if (actualPrintingSize.height() < availablePrintingRect.height())
	{
		// Center vertically
		qreal deltay = (availablePrintingRect.height() - actualPrintingSize.height()) / 2;
		actualPrintingRect.translate(0, deltay);
	}
	else
	{
		// Center horizontally
		qreal deltax = (availablePrintingRect.width() - actualPrintingRect.width()) / 2;
		actualPrintingRect.translate(deltax, 0);
	}

	return actualPrintingRect;
}

void MachineImageExporter::freeRenderingResources()
{
	this->painter.reset();
	this->pixmap.reset();
	this->generator.reset();
	this->printer.reset();
}
