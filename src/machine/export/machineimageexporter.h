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

#ifndef MACHINEIMAGEEXPORTER_H
#define MACHINEIMAGEEXPORTER_H

// Parent
#include <QObject>

// Stdlib
#include <memory>
using namespace std;

// Qt
#include <QRectF>
#include <QPageLayout>
class QPaintDevice;
class QPainter;

// StateS
#include "statestypes.h"
#include "variabletablescene.h"
#include "componentscene.h"


class MachineImageExporter : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class ImageFormat_t { pdf, svg, png, jpg };
	enum class LeftRight_t   { left, right };

	/////
	// Constructors/destructors
public:
	explicit MachineImageExporter(StatesScene* stateGraphScene) :
		stateGraphScene(stateGraphScene                  ),
		componentScene (make_shared<ComponentScene>()    ),
		variableScene  (make_shared<VariableTableScene>())
	{}

	/////
	// Object functions
public:
	void setImageFormat(ImageFormat_t imageFormat);

	void setVectorPageLayout(const QPageLayout& pageLayout);
	void setBitmapSize(const QSize& size);

	void setDisplayComponent(bool doDisplay);
	void setDisplayInputs   (bool doDisplay);
	void setDisplayOutputs  (bool doDisplay);
	void setDisplayVariables(bool doDisplay);
	void setDisplayConstants(bool doDisplay);
	void setDisplayBorder   (bool doDisplay);

	void setOuterMargin(int margin);
	void setInnerMargin(int margin);

	void setStateGraphRatio(uint stateGraphRatio);

	void setInfoPosition(LeftRight_t infoPosition);

	shared_ptr<QPixmap> renderPreview(uint sideInPixels);
	void doExport(const QString& path);

private:
	void renderPdf(const QString& path, const QString& title, const QString& creator, QPageLayout pageLayoutWithMargin);
	void renderSvg(const QString& path, const QString& title, const QString& creator, QPageLayout pageLayoutWithMargin);
	shared_ptr<QPixmap> renderBitmap(qreal width, qreal height);

	void generatePrintingRects(QRectF renderAreaRect);

	void renderOnDevice(QPaintDevice* device, uint bitmapPenWidth = 0);

	void renderStateGraph(QPainter* painter);
	void renderComponent (QPainter* painter);
	void renderVariables (QPainter* painter);
	void renderBorder    (QPainter* painter);

	QRectF getActualPrintingRect(const QRectF& inputSceneRect, const QRectF& availablePrintingRect) const;

	bool hasVariablesToDisplay() const;

	/////
	// Object variables
private:
	// Input scenes
	StatesScene*                   stateGraphScene;
	shared_ptr<StatesScene>        componentScene;
	shared_ptr<VariableTableScene> variableScene;

	// Configuration
	ImageFormat_t format = ImageFormat_t::svg;

	QPageLayout vectorPageLayout;
	QSize       bitmapSize;

	bool displayComponent = false;
	bool displayInputs    = false;
	bool displayOutputs   = false;
	bool displayVariables = false;
	bool displayConstants = false;
	bool displayBorder    = false;

	int outerMargin = 3;
	int innerMargin = 3;

	uint stateGraphRatio = 3;

	LeftRight_t infoPosition = LeftRight_t::right;

	// Rects indicating area available for rendering
	QRectF stateGraphRect;
	QRectF componentRect;
	QRectF variablesRect;

};

#endif // MACHINEIMAGEEXPORTER_H
