/*
 * Copyright © 2014-2026 Clément Foucher
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
#include <QSvgGenerator>

// StateS classes
#include "states.h"
#include "machinemanager.h"
#include "machine.h"


void MachineImageExporter::setImageFormat(ImageFormat_t imageFormat)
{
	this->format = imageFormat;
}

void MachineImageExporter::setVectorPageLayout(const QPageLayout& pageLayout)
{
	this->vectorPageLayout = pageLayout;
}

void MachineImageExporter::setBitmapSize(const QSize& size)
{
	this->bitmapSize = size;
}

void MachineImageExporter::setDisplayComponent(bool doDisplay)
{
	this->displayComponent = doDisplay;
}

void MachineImageExporter::setDisplayInputs(bool doDisplay)
{
	this->displayInputs = doDisplay;
}

void MachineImageExporter::setDisplayOutputs(bool doDisplay)
{
	this->displayOutputs = doDisplay;
}

void MachineImageExporter::setDisplayVariables(bool doDisplay)
{
	this->displayVariables = doDisplay;
}

void MachineImageExporter::setDisplayConstants(bool doDisplay)
{
	this->displayConstants = doDisplay;
}

void MachineImageExporter::setDisplayBorder(bool doDisplay)
{
	this->displayBorder = doDisplay;
}

void MachineImageExporter::setOuterMargin(int margin)
{
	this->outerMargin = margin;
}

void MachineImageExporter::setInnerMargin(int margin)
{
	this->innerMargin = margin;
}

void MachineImageExporter::setStateGraphRatio(uint stateGraphRatio)
{
	this->stateGraphRatio = stateGraphRatio;
}

void MachineImageExporter::setInfoPosition(LeftRight_t infoPosition)
{
	this->infoPosition = infoPosition;
}

shared_ptr<QPixmap> MachineImageExporter::renderPreview(uint sideInPixels)
{
	// Get current width and height
	qreal width;
	qreal height;
	switch (this->format)
	{
	case ImageFormat_t::pdf:
	case ImageFormat_t::svg:
	{
		auto pageFullRect = this->vectorPageLayout.fullRect();
		width  = pageFullRect.width();
		height = pageFullRect.height();
		break;
	}
	case ImageFormat_t::jpg:
	case ImageFormat_t::png:
		width  = this->bitmapSize.width();
		height = this->bitmapSize.height();
		break;
	}

	// Determine ratio
	qreal ratio = width/height;

	// Compute a rectangle that has the same ratio and
	// fits in a square with the given side size
	if (ratio > 1)
	{
		width  = sideInPixels;
		height = width/ratio;
	}
	else
	{
		height = sideInPixels;
		width = height*ratio;
	}

	return this->renderBitmap(width, height);
}

void MachineImageExporter::doExport(const QString& path)
{
	if ( (this->format == ImageFormat_t::pdf) || (this->format == ImageFormat_t::svg) )
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return;


		QString vectorComment = tr("Created with") + " StateS v." + StateS::getVersion();

		QPageLayout pageLayoutWithMargin = this->vectorPageLayout;
		pageLayoutWithMargin.setMode(QPageLayout::Mode::StandardMode);
		auto pageFullRect = pageLayoutWithMargin.fullRect();

		qreal rawMarginH = pageFullRect.width() *this->outerMargin/100;
		qreal rawMarginV = pageFullRect.height()*this->outerMargin/100;
		QMarginsF margins(rawMarginH, rawMarginV, rawMarginH, rawMarginV);

		pageLayoutWithMargin.setMargins(margins);

		if (this->format == ImageFormat_t::pdf)
		{
			this->renderPdf(path, machine->getName(), vectorComment, pageLayoutWithMargin);
		}
		else // svg
		{
			this->renderSvg(path, machine->getName(), vectorComment, pageLayoutWithMargin);
		}
	}
	else // Bitmap
	{
		auto pixmap = this->renderBitmap(this->bitmapSize.width(), this->bitmapSize.height());
		pixmap->save(path);
	}
}

void MachineImageExporter::renderPdf(const QString& path, const QString& title, const QString& creator, QPageLayout pageLayoutWithMargin)
{
	// Build and configure PDF printer
	auto pdfPrinter = make_shared<QPrinter>(QPrinter::HighResolution);
	pdfPrinter->setOutputFormat(QPrinter::PdfFormat);
	pdfPrinter->setPageLayout(pageLayoutWithMargin);
	pdfPrinter->setOutputFileName(path);
	pdfPrinter->setDocName(title);
	pdfPrinter->setCreator(creator);

	// Compute printing rects
	auto paintRectInInches = pdfPrinter->pageRect(QPrinter::Unit::Inch);
	qreal resolutionInDotsPerInches = pdfPrinter->resolution();
	QRectF renderAreaRect(// Printer origin is the top left corner of the page rect
	                      0, 0,
	                      // Size of the rect in dots
	                      paintRectInInches.width()*resolutionInDotsPerInches, paintRectInInches.height()*resolutionInDotsPerInches
	                     );
	this->generatePrintingRects(renderAreaRect);

	// Render
	this->renderOnDevice(pdfPrinter.get());
}

void MachineImageExporter::renderSvg(const QString& path, const QString& title, const QString& creator, QPageLayout pageLayoutWithMargin)
{
	// Build and configure SVG generator
	auto svgGenerator = make_shared<QSvgGenerator>();
	auto pageFullRect = pageLayoutWithMargin.fullRect(QPageLayout::Unit::Point);
	svgGenerator->setSize(pageFullRect.size().toSize());
	svgGenerator->setViewBox(pageFullRect);
	svgGenerator->setFileName(path);
	svgGenerator->setTitle(title);
	svgGenerator->setDescription(creator);

	// Compute printing rects
	auto paintRect = pageLayoutWithMargin.paintRect(QPageLayout::Unit::Point);
	this->generatePrintingRects(paintRect);

	// Render
	this->renderOnDevice(svgGenerator.get());
}

shared_ptr<QPixmap> MachineImageExporter::renderBitmap(qreal width, qreal height)
{
	// Build and fill pixmap
	auto pixmap = make_shared<QPixmap>(width, height);
	pixmap->fill();

	// Compute printing rects
	qreal rawMarginH = width*this->outerMargin/100;
	qreal rawMarginV = height*this->outerMargin/100;
	QRectF renderAreaRect(rawMarginH, rawMarginV, width-2*rawMarginH, height-2*rawMarginV);
	this->generatePrintingRects(renderAreaRect);

	// Compute bitmap pen size (used for border) to be 0.1 % of the image size (min 1 pixel)
	int bitmapPenWidth = min(width, height)*0.1/100;
	if (bitmapPenWidth < 1)
	{
		bitmapPenWidth = 1;
	}

	// Render
	this->renderOnDevice(pixmap.get(), bitmapPenWidth);

	return pixmap;
}

/**
 * @brief MachineImageExporter::generatePrintingRects generates the
 * printing rect for each item based on the renderAreaRect, which is
 * the available drawing area (i.e. total output area minus borders)
 */
void MachineImageExporter::generatePrintingRects(QRectF renderAreaRect)
{
	this->stateGraphRect = renderAreaRect;
	this->componentRect  = QRectF();
	this->variablesRect  = QRectF();

	if ( (this->displayComponent == true) || (this->hasVariablesToDisplay() == true) )
	{
		const qreal availableHeight = renderAreaRect.height();
		const qreal availableWidth  = renderAreaRect.width();

		const qreal infoWidth  = (  availableWidth                        / (this->stateGraphRatio+1) );
		const qreal sceneWidth = ( (availableWidth*this->stateGraphRatio) / (this->stateGraphRatio+1) );

		const qreal infoLeftPos = (this->infoPosition == LeftRight_t::left) ? renderAreaRect.left() : renderAreaRect.left() + sceneWidth;

		//
		// State graph

		// Reduce state graph width
		this->stateGraphRect.setWidth(sceneWidth);

		// Set horizontal position
		if (this->infoPosition == LeftRight_t::left)
		{
			this->stateGraphRect.translate(infoWidth, 0);
		}

		//
		// Component

		if (this->displayComponent == true)
		{
			const qreal rectTopPos = renderAreaRect.top();
			const qreal rectHeight = (this->hasVariablesToDisplay() == false) ? availableHeight : availableHeight/2;

			this->componentRect = QRectF(infoLeftPos, rectTopPos, infoWidth, rectHeight);
		}

		//
		// Variables

		if (this->hasVariablesToDisplay() == true)
		{
			const qreal rectTopPos = (this->displayComponent == false) ? renderAreaRect.top() : renderAreaRect.top() + availableHeight/2;
			const qreal rectHeight = (this->displayComponent == false) ? availableHeight      : availableHeight/2;

			this->variablesRect = QRectF(infoLeftPos, rectTopPos, infoWidth, rectHeight);
		}
	}
}

/**
 * @brief MachineImageExporter::renderOnDevice does the rendering
 *        on the provided device.
 *        These member variables must have been computed before:
 *        -	stateGraphRect
 *        - componentRect (only if component is to be rendered)
 *        - variablesRect (only if variables are to be rendered)
 * @param device Device on which to do the rendering.
 * @param bitmapPenWidth Width of the pen used to render the border for bitmap rendering.
 */
void MachineImageExporter::renderOnDevice(QPaintDevice* device, uint bitmapPenWidth)
{
	QPainter painter(device);

	// For bitmap rendering
	if (bitmapPenWidth != 0)
	{
		QBrush brush(Qt::black);
		QPen bitmapPen(brush, bitmapPenWidth);
		painter.setPen(bitmapPen);

		painter.setRenderHint(QPainter::Antialiasing);
	}

	this->renderStateGraph(&painter);
	this->renderComponent (&painter);
	this->renderVariables (&painter);
	this->renderBorder    (&painter);
}

void MachineImageExporter::renderStateGraph(QPainter* painter)
{
	if (painter == nullptr) return;

	if (this->stateGraphRect.isNull() == true) return;

	if (this->stateGraphScene == nullptr) return;


	// Get rects
	auto itemsBoundingRect  = this->stateGraphScene->getItemsBoundingRect();
	auto actualPrintingRect = this->getActualPrintingRect(itemsBoundingRect, this->stateGraphRect);

	// Render
	this->stateGraphScene->render(painter, actualPrintingRect, itemsBoundingRect);
}

void MachineImageExporter::renderComponent(QPainter* painter)
{
	if (this->displayComponent == false) return;

	if (painter == nullptr) return;

	if (this->componentRect.isNull() == true) return;

	if (this->componentScene == nullptr) return;


	// Get rects
	auto itemsBoundingRect  = this->componentScene->getItemsBoundingRect();
	auto actualPrintingRect = this->getActualPrintingRect(itemsBoundingRect, this->componentRect);

	// Render
	this->componentScene->render(painter, actualPrintingRect, itemsBoundingRect);
}

void MachineImageExporter::renderVariables(QPainter* painter)
{
	if (this->hasVariablesToDisplay() == false) return;

	if (painter == nullptr) return;

	if (this->variablesRect.isNull() == true) return;


	// Build scene
	this->variableScene->buildScene(this->displayInputs, this->displayOutputs, this->displayVariables, this->displayConstants);

	// Get rects
	auto itemsBoundingRect  = variableScene->getItemsBoundingRect();
	auto actualPrintingRect = this->getActualPrintingRect(itemsBoundingRect, this->variablesRect);

	// Render
	variableScene->render(painter, actualPrintingRect, itemsBoundingRect);
}

void MachineImageExporter::renderBorder(QPainter* painter)
{
	if (this->displayBorder == false) return;

	if (painter == nullptr) return;


	if (this->stateGraphRect.isNull() == false)
	{
		painter->drawRect(this->stateGraphRect);
	}

	if (this->componentRect.isNull() == false)
	{
		painter->drawRect(this->componentRect);
	}

	if (this->variablesRect.isNull() == false)
	{
		painter->drawRect(this->variablesRect);
	}
}

/**
 * @brief MachineImageExporter::getActualPrintingRect computes the optimal
 *        printing rect for a scene. Given an available rect, it will
 *        take a subrect that has the correct ratio for displaying the
 *        input scene rect. It will alse add the inner margin if requested.
 *        The output rect is centered on the available printing rect.
 * @param inputSceneRect Rect containing the part of the scene to be rendered.
 * @param availablePrintingRect Rect representing the available rect for
 *        rendering this scene on the output device.
 * @return A subrect of availablePrintingRect, that has the same ratio
 *         as the inputSceneRect, that respect inner margin and that is
 *         centered on the availablePrintingRect.
 */
QRectF MachineImageExporter::getActualPrintingRect(const QRectF& inputSceneRect, const QRectF& availablePrintingRect) const
{
	// Get the size of the available area
	auto availablePrintingSize = availablePrintingRect.size();

	// Add inner margin if there is a border
	if ( (this->displayBorder == true) && (this->innerMargin > 0) )
	{
		qreal hMargin = availablePrintingRect.width() *this->innerMargin/100;
		qreal vMargin = availablePrintingRect.height()*this->innerMargin/100;
		auto margins = QMarginsF(hMargin, vMargin, hMargin, vMargin);

		availablePrintingSize = availablePrintingSize.shrunkBy(margins);
	}

	// Determine the actual printing size that preserves the drawing ratio
	auto actualPrintingSize = inputSceneRect.size();
	actualPrintingSize.scale(availablePrintingSize, Qt::KeepAspectRatio);

	// Build the printing rect and center it over the printing area
	QRectF actualPrintingRect(QPointF(0,0), actualPrintingSize);
	actualPrintingRect.moveCenter(availablePrintingRect.center());

	return actualPrintingRect;
}

bool MachineImageExporter::hasVariablesToDisplay() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;


	if ( (this->displayInputs == true) && (machine->getInputVariablesIds().empty() == false) )
	{
		return true;
	}
	else if ( (this->displayOutputs == true) && (machine->getOutputVariablesIds().empty() == false) )
	{
		return true;
	}
	else if ( (this->displayVariables == true) && (machine->getInternalVariablesIds().empty() == false) )
	{
		return true;
	}
	else if ( (this->displayConstants == true) && (machine->getConstantsIds().empty() == false) )
	{
		return true;
	}
	else
	{
		return false;
	}
}
