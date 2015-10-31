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
#include "imageexportdialog.h"

// Qt classes
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QFileDialog>

// StateS classes
#include <checkboxhtml.h>


ImageExportDialog::ImageExportDialog(const QString& baseFileName, const QString& searchPath, QWidget *parent) :
    QDialog(parent)
{
    this->baseFileName = baseFileName;
    this->searchPath   = searchPath;

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* title = new QLabel("<b>" + tr("Choose image options:") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QFormLayout* formLayout = new QFormLayout();
    this->imageFormatSelectionBox = new QComboBox();
    this->imageFormatSelectionBox->addItem("Pdf");
    this->imageFormatSelectionBox->addItem("Svg");
    this->imageFormatSelectionBox->addItem("Png");
    this->imageFormatSelectionBox->addItem("Jpeg");
    formLayout->addRow(tr("Image format:"), this->imageFormatSelectionBox);
    layout->addLayout(formLayout);

    this->includeComponentCheckBox = new CheckBoxHtml(tr("Include component external view:"));
    layout->addWidget(this->includeComponentCheckBox);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout);

    QPushButton* buttonOK = new QPushButton(tr("OK"));
    connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
    buttonsLayout->addWidget(buttonOK);

    QPushButton* buttonCancel = new QPushButton(tr("Cancel"));
    connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
    buttonsLayout->addWidget(buttonCancel);
}

bool ImageExportDialog::includeComponent()
{
    return this->includeComponentCheckBox->isChecked();
}

MachineImageExporter::imageFormat ImageExportDialog::getImageFormat()
{
    if (this->imageFormatSelectionBox->currentText() == "Pdf")
        return MachineImageExporter::imageFormat::pdf;
    else if (this->imageFormatSelectionBox->currentText() == "Svg")
        return MachineImageExporter::imageFormat::svg;
    else if (this->imageFormatSelectionBox->currentText() == "Png")
        return MachineImageExporter::imageFormat::png;
    else // if (this->imageFormatSelectionBox->currentText() == "Jpeg")
        return MachineImageExporter::imageFormat::jpg;
}

QString ImageExportDialog::getFilePath()
{
    return this->filePath;
}

void ImageExportDialog::accept()
{
    MachineImageExporter::imageFormat format = this->getImageFormat();

    QString defaultFilePath;

    if (! this->searchPath.isEmpty())
    {
        defaultFilePath += this->searchPath;
        defaultFilePath += "/"; // TODO: check if environment dependant!
    }

    defaultFilePath += this->baseFileName;

    this->filePath = QString::null;

    switch(format)
    {
    case MachineImageExporter::imageFormat::pdf:
        this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Pdf"), defaultFilePath + ".pdf", "*.pdf");

        if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".pdf", Qt::CaseInsensitive)) )
            this->filePath += ".pdf";
        break;
    case MachineImageExporter::imageFormat::svg:
        this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Svg"), defaultFilePath + ".svg", "*.svg");

        if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".svg", Qt::CaseInsensitive)) )
            this->filePath += ".svg";
        break;
    case MachineImageExporter::imageFormat::png:
        this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Png"), defaultFilePath + ".png", "*.png");

        if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".png", Qt::CaseInsensitive)) )
            this->filePath += ".png";
        break;
    case MachineImageExporter::imageFormat::jpg:
        this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Jpeg"), defaultFilePath + ".jpg", "*.jpg");

        if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".jpg", Qt::CaseInsensitive)) )
            this->filePath += ".jpg";

        break;
    }

    if (! this->filePath.isEmpty())
        QDialog::accept();
}

