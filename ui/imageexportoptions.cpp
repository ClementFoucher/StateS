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
#include "imageexportoptions.h"

// Qt classes
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>


ImageExportOptions::ImageExportOptions(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* title = new QLabel("<b>" + tr("Choose image options:") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QFormLayout* formLayout = new QFormLayout();
    layout->addLayout(formLayout);

    this->imageFormatSelectionBox = new QComboBox();
    this->imageFormatSelectionBox->addItem("Pdf");
    this->imageFormatSelectionBox->addItem("Svg");
    this->imageFormatSelectionBox->addItem("Png");
    this->imageFormatSelectionBox->addItem("Jpeg");
    formLayout->addRow(tr("Image format:"), this->imageFormatSelectionBox);

    this->includeComponentCheckBox = new QCheckBox();
    formLayout->addRow(tr("Include component external view:"), this->includeComponentCheckBox);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout);

    QPushButton* buttonOK = new QPushButton(tr("OK"));
    connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
    buttonsLayout->addWidget(buttonOK);

    QPushButton* buttonCancel = new QPushButton(tr("Cancel"));
    connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
    buttonsLayout->addWidget(buttonCancel);
}

bool ImageExportOptions::includeComponent()
{
    return this->includeComponentCheckBox->isChecked();
}

ImageExportOptions::imageFormat ImageExportOptions::getImageFormat()
{
    if (this->imageFormatSelectionBox->currentText() == "Pdf")
        return ImageExportOptions::imageFormat::pdf;
    else if (this->imageFormatSelectionBox->currentText() == "Svg")
        return ImageExportOptions::imageFormat::svg;
    else if (this->imageFormatSelectionBox->currentText() == "Png")
        return ImageExportOptions::imageFormat::png;
    else // if (this->imageFormatSelectionBox->currentText() == "Jpeg")
        return ImageExportOptions::imageFormat::jpg;
}

