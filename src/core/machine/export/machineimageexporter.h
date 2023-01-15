/*
 * Copyright © 2014-2023 Clément Foucher
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

#ifndef MACHINEIMAGEEXPORTER_H
#define MACHINEIMAGEEXPORTER_H

// Parent
#include <QObject>

// C++ classes
using namespace std;
#include <memory>

// Qt classes
#include <QRectF>
class QGraphicsScene;
class QPainter;
class QPrinter;
class QSvgGenerator;

// StateS classes
#include "statestypes.h"
class GenericScene;


class MachineImageExporter : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineImageExporter(GenericScene* scene, shared_ptr<QGraphicsScene> component);

	/////
	// Object functions
public:
	void setDisplayComponent(bool doDisplay);
	void setDisplayConstants(bool doDisplay);
	void setDisplayVariables(bool doDisplay);
	void setDisplayBorder(bool doDisplay);
	void setMainSceneRatio(uint sceneRatio);
	void setInfoPos(LeftRight_t pos);

	shared_ptr<QPixmap> renderPreview(QSizeF previewSize);
	void doExport(const QString& path, ImageFormat_t format, const QString& creator = QString()); // TODO: throw StatesException for file access

private:
	void generatePrintingRects();
	void preparePdfPrinter(const QString& path, const QString& title, const QString& creator);
	void renderPdf();
	void renderSvg(const QString& path, const QString& title, const QString& creator);
	void renderBitmap();

	void renderOnPainter();

	void renderScene();
	void renderComponent();
	void renderConstants();
	void renderVariables();
	void prepareBorder(const QRectF& availablePrintingRect);
	void renderBorder();

	QRectF getActualPrintedRect(const QRectF& elementPrintingRect, const QRectF& availablePrintingRect);

	void freeRenderingResources();

	/////
	// Object variables
private:
	const qreal spacer = 50;

	// Required elements
	GenericScene* scene;
	weak_ptr<QGraphicsScene> component;
	shared_ptr<QGraphicsScene> border;

	// Options
	uint mainSceneRatio;
	bool includeComponent;
	bool includeConstant;
	bool includeVariables;
	bool addBorder;
	LeftRight_t infoPosition = LeftRight_t::left;
	bool strictBorders;

	// There objects handle rendering on file. All must be persistent until export is over
	shared_ptr<QPrinter>      printer;
	shared_ptr<QSvgGenerator> generator;
	shared_ptr<QPixmap>       pixmap;

	// Object we paint on
	shared_ptr<QPainter> painter;

	// Printing areas
	QRectF pageRect;
	QRectF renderAreaWithoutBordersRect;
	QRectF scenePrintingRect;
	QRectF componentPrintingRect;
	QRectF constantsPrintingRect;
	QRectF variablesPrintingRect;

};

#endif // MACHINEIMAGEEXPORTER_H
