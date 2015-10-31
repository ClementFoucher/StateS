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
#include "machineimageexporter.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QPainter>
#include <QPrinter>
#include <QSvgGenerator>
#include <QGraphicsScene>


void MachineImageExporter::exportMachineAsImage(const QString& path, const QString& title, const QString& creator, MachineImageExporter::imageFormat format, QGraphicsScene* scene, QGraphicsScene* component)
{
    // There objects handle rendenring on file
    unique_ptr<QPrinter>      printer   = nullptr;
    unique_ptr<QSvgGenerator> generator = nullptr;
    unique_ptr<QPixmap>       pixmap    = nullptr;

    // Object we paint on
    unique_ptr<QPainter> painter = nullptr;

    // Printing areas
    QRectF componentPrintingRect;
    QRectF scenePrintingRect;

    QRectF sceneRect = scene->sceneRect();
    int splitter = 50;

    int n = 3; // This factor indicates scene's width will be n times component's width.

    if (format == imageFormat::pdf)
    {
        // The n factor gives an area occupation of (1/(n+1)) for component and (n/(n+1)) for scene.
        int N = n + 1; // This is the total number of areas in page

        printer = unique_ptr<QPrinter>(new QPrinter(QPrinter::HighResolution));
        printer->setOutputFileName(path);

        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setPageSize(QPrinter::A4);
        printer->setPageOrientation(QPageLayout::Landscape);

        printer->setDocName(title);
        printer->setCreator(creator);

        painter = unique_ptr<QPainter>(new QPainter());
        painter->begin(printer.get());

        if (component != nullptr)
        {
            int allowedPageHeight = printer->pageRect().height();
            int allowedPageWidth  = printer->pageRect().width() - splitter;

            componentPrintingRect.setWidth(allowedPageWidth / N);
            componentPrintingRect.setHeight(allowedPageHeight);

            scenePrintingRect.setWidth( (n*allowedPageWidth) / N );
            scenePrintingRect.setHeight(allowedPageHeight);

            scenePrintingRect.moveTopLeft(QPointF(componentPrintingRect.width() + splitter, 0));
        }
        else
        {
            // TODO: center on page
            scenePrintingRect.setSize(printer->pageRect().size());
        }
    }
    else
    {
        QSize totalPrintedSize;

        if (component != nullptr)
        {
            // Make component a size of 1/nth the machine
            componentPrintingRect.setWidth(sceneRect.width() / n);
            componentPrintingRect.setHeight(sceneRect.height());
            componentPrintingRect.moveTopLeft(QPointF(splitter, splitter));

            scenePrintingRect = sceneRect;
            scenePrintingRect.moveTopLeft(QPointF(componentPrintingRect.width() + splitter, splitter));

            totalPrintedSize = QSize(componentPrintingRect.width() + 3 * splitter + scenePrintingRect.width(), scenePrintingRect.size().height() + 2 * splitter);
        }
        else
        {
            scenePrintingRect = sceneRect;
            scenePrintingRect.moveTopLeft(QPointF(splitter, splitter));
            totalPrintedSize = QSize(scenePrintingRect.width() + 3 * splitter, scenePrintingRect.height() + 2 * splitter);
        }

        if (format == imageFormat::svg)
        {
            generator = unique_ptr<QSvgGenerator>(new QSvgGenerator());
            generator->setFileName(path);

            generator->setSize(totalPrintedSize);

            generator->setTitle(title);
            generator->setDescription(creator);

            painter = unique_ptr<QPainter>(new QPainter());
            painter->begin(generator.get());

        }
        else if (format == imageFormat::png)
        {
            pixmap = unique_ptr<QPixmap>(new QPixmap(totalPrintedSize));

            pixmap->fill();

            painter = unique_ptr<QPainter>(new QPainter());
            painter->begin(pixmap.get());
            painter->setRenderHint(QPainter::Antialiasing);
        }
        else if (format == imageFormat::jpg)
        {
            pixmap = unique_ptr<QPixmap>(new QPixmap(totalPrintedSize));

            pixmap->fill();

            painter = unique_ptr<QPainter>(new QPainter());
            painter->begin(pixmap.get());
            painter->setRenderHint(QPainter::Antialiasing);
        }
    }

    if (painter != nullptr)
    {
        // Paint scene
        scene->render(painter.get(), scenePrintingRect);

        if (component != nullptr)
        {
            // Paint component
            component->render(painter.get(), componentPrintingRect);
        }

        // Done
        painter->end();
    }

    // For pixmap exporters, need to save.
    if (pixmap != nullptr)
    {
        pixmap->save(path);
    }
}
