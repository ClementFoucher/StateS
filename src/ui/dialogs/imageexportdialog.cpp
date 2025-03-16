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
#include "imageexportdialog.h"

// Qt classes
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QFileDialog>

// StateS classes
#include "checkboxhtml.h"
#include "machineimageexporter.h"


ImageExportDialog::ImageExportDialog(const QString& baseFileName, shared_ptr<MachineImageExporter> imageExporter, const QString& searchPath, QWidget* parent) :
	QDialog(parent)
{
	this->baseFileName = baseFileName;
	this->searchPath   = searchPath;

	this->previewManager = imageExporter;

	this->previewPixmap = this->previewManager->renderPreview(QSizeF(200, 200));
	this->previewWidget = new QLabel();
	this->previewWidget->setMinimumSize(200, 200);
	this->previewWidget->setPixmap(*this->previewPixmap.get());

	this->setWindowTitle(tr("Image export"));

	QVBoxLayout* layout = new QVBoxLayout(this);

	QLabel* title = new QLabel("<b>" + tr("Customize image") + "</b>");
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);

	QFormLayout* formLayout = new QFormLayout();
	this->imageFormatSelectionBox = new QComboBox();
	this->imageFormatSelectionBox->addItem("Svg");
	this->imageFormatSelectionBox->addItem("Pdf");
	this->imageFormatSelectionBox->addItem("Png");
	this->imageFormatSelectionBox->addItem("Jpeg");
	formLayout->addRow(tr("Image format"), this->imageFormatSelectionBox);
	layout->addLayout(formLayout);

	this->includeComponentCheckBox = new CheckBoxHtml(tr("Include component external view"));
	connect(this->includeComponentCheckBox, &CheckBoxHtml::toggled, this, &ImageExportDialog::includeComponentCheckBoxChanged);
	this->includeComponentCheckBox->setChecked(true);
	layout->addWidget(this->includeComponentCheckBox);

	this->includeConstantsCheckBox = new CheckBoxHtml(tr("Include constants"));
	connect(this->includeConstantsCheckBox, &CheckBoxHtml::toggled, this, &ImageExportDialog::includeConstantsCheckBoxChanged);
	layout->addWidget(this->includeConstantsCheckBox);

	this->includeVariablesCheckBox = new CheckBoxHtml(tr("Include variables"));
	connect(this->includeVariablesCheckBox, &CheckBoxHtml::toggled, this, &ImageExportDialog::includeVariablesCheckBoxChanged);
	layout->addWidget(this->includeVariablesCheckBox);

	this->infoToTheRightCheckBox = new CheckBoxHtml(tr("Place information to the right"));
	connect(this->infoToTheRightCheckBox, &CheckBoxHtml::toggled, this, &ImageExportDialog::infoToTheRightCheckBoxChanged);
	layout->addWidget(this->infoToTheRightCheckBox);

	this->addBorderCheckBox = new CheckBoxHtml(tr("Add border"));
	connect(this->addBorderCheckBox, &CheckBoxHtml::toggled, this, &ImageExportDialog::addBorderCheckBoxChanged);
	layout->addWidget(this->addBorderCheckBox);

	QHBoxLayout* sliderLayout = new QHBoxLayout();
	QLabel* sliderLabel = new QLabel(tr("Ratio between machine view and aditional information"));
	sliderLayout->addWidget(sliderLabel);
	this->ratioSlider = new QSlider(Qt::Horizontal);
	this->ratioSlider->setMinimum(1);
	this->ratioSlider->setMaximum(10);
	this->ratioSlider->setValue(3);
	connect(this->ratioSlider, &QSlider::valueChanged, this, &ImageExportDialog::ratioSliderValueChanged);
	sliderLayout->addWidget(this->ratioSlider);
	layout->addLayout(sliderLayout);

	QLabel* previewTitle = new QLabel("<b>" + tr("Preview") + "</b>");
	previewTitle->setAlignment(Qt::AlignCenter);
	layout->addWidget(previewTitle);

	layout->addWidget(this->previewWidget);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	layout->addLayout(buttonsLayout);

	QPushButton* buttonOK = new QPushButton(tr("OK"));
	connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
	buttonsLayout->addWidget(buttonOK);

	QPushButton* buttonCancel = new QPushButton(tr("Cancel"));
	connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
	buttonsLayout->addWidget(buttonCancel);
}

ImageFormat_t ImageExportDialog::getImageFormat() const
{
	if (this->imageFormatSelectionBox->currentText() == "Pdf")
	{
		return ImageFormat_t::pdf;
	}
	else if (this->imageFormatSelectionBox->currentText() == "Svg")
	{
		return ImageFormat_t::svg;
	}
	else if (this->imageFormatSelectionBox->currentText() == "Png")
	{
		return ImageFormat_t::png;
	}
	else // if (this->imageFormatSelectionBox->currentText() == "Jpeg")
	{
		return ImageFormat_t::jpg;
	}
}

QString ImageExportDialog::getFilePath() const
{
	return this->filePath;
}

shared_ptr<MachineImageExporter> ImageExportDialog::getImageExporter() const
{
	return this->previewManager;
}

void ImageExportDialog::accept()
{
	ImageFormat_t format = this->getImageFormat();

	QString defaultFilePath;

	if (this->searchPath.isEmpty() == false)
	{
		defaultFilePath += this->searchPath;
		defaultFilePath += "/"; // TODO: check if environment dependant!
	}

	defaultFilePath += this->baseFileName;

	this->filePath = QString();

	switch(format)
	{
	case ImageFormat_t::pdf:
		this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Pdf"), defaultFilePath + ".pdf", "*.pdf");

		if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".pdf", Qt::CaseInsensitive)) )
		{
			this->filePath += ".pdf";
		}
		break;
	case ImageFormat_t::svg:
		this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Svg"), defaultFilePath + ".svg", "*.svg");

		if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".svg", Qt::CaseInsensitive)) )
		{
			this->filePath += ".svg";
		}
		break;
	case ImageFormat_t::png:
		this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Png"), defaultFilePath + ".png", "*.png");

		if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".png", Qt::CaseInsensitive)) )
		{
			this->filePath += ".png";
		}
		break;
	case ImageFormat_t::jpg:
		this->filePath = QFileDialog::getSaveFileName(this, tr("Export machine to Jpeg"), defaultFilePath + ".jpg", "*.jpg");

		if ( (! this->filePath.isEmpty()) && (! this->filePath.endsWith(".jpg", Qt::CaseInsensitive)) )
		{
			this->filePath += ".jpg";
		}
		break;
	}

	if (this->filePath.isEmpty() == false)
	{
		QDialog::accept();
	}
}

void ImageExportDialog::resizeEvent(QResizeEvent*)
{
	this->updatePreview();
}

void ImageExportDialog::includeComponentCheckBoxChanged(bool b)
{
	this->previewManager->setDisplayComponent(b);
	this->updatePreview();
}

void ImageExportDialog::includeConstantsCheckBoxChanged(bool b)
{
	this->previewManager->setDisplayConstants(b);
	this->updatePreview();
}

void ImageExportDialog::includeVariablesCheckBoxChanged(bool b)
{
	this->previewManager->setDisplayVariables(b);
	this->updatePreview();
}

void ImageExportDialog::infoToTheRightCheckBoxChanged(bool b)
{
	this->previewManager->setInfoPos(b ? LeftRight_t::right : LeftRight_t::left);
	this->updatePreview();
}

void ImageExportDialog::addBorderCheckBoxChanged(bool b)
{
	this->previewManager->setDisplayBorder(b);
	this->updatePreview();
}

void ImageExportDialog::ratioSliderValueChanged(int i)
{
	this->previewManager->setMainSceneRatio(i);
	this->updatePreview();
}

void ImageExportDialog::updatePreview()
{
	this->previewPixmap = this->previewManager->renderPreview(QSizeF(this->previewWidget->width(), this->previewWidget->height()));
	this->previewWidget->setPixmap(*this->previewPixmap.get());
}
