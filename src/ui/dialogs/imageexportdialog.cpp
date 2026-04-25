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
#include "imageexportdialog.h"

// Qt classes
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>

// StateS classes
#include "machineimageexporter.h"
#include "documentsizeeditor.h"
#include "sliderwithtitleandvalue.h"
#include "savefiledialog.h"


ImageExportDialog::ImageExportDialog(StatesScene* stateGraphScene, const QString& baseFilePath, const QString& baseFileName, QWidget* parent) :
	StatesDialog(parent)
{
	this->imageExporter = make_shared<MachineImageExporter>(stateGraphScene);
	this->baseFilePath  = baseFilePath;
	this->baseFileName  = baseFileName;

	this->setWindowTitle(tr("Export as image"));

	//
	// Build widgets

	// Main configuration group box

	auto mainConfigurationGroup = new QGroupBox(tr("Customize image"));

	auto imageFormatLabel = new QLabel(tr("Format:"));

	this->imageFormatComboBox = new QComboBox();
	this->imageFormatComboBox->addItem("Svg");
	this->imageFormatComboBox->addItem("Pdf");
	this->imageFormatComboBox->addItem("Png");
	this->imageFormatComboBox->addItem("Jpeg");
	this->imageFormatComboBox->setItemData(0, static_cast<int>(ImageFormat_t::svg));
	this->imageFormatComboBox->setItemData(1, static_cast<int>(ImageFormat_t::pdf));
	this->imageFormatComboBox->setItemData(2, static_cast<int>(ImageFormat_t::png));
	this->imageFormatComboBox->setItemData(3, static_cast<int>(ImageFormat_t::jpg));

	this->imageSizeEditor = new DocumentSizeEditor();

	auto outerMarginSlider = new SliderWithTitleAndValue(0, 25, 3,
	                                                     tr("Margin:"),
	                                                     "", " %"
	                                                    );

	auto includeAdditionalInfoCheckBox = new QCheckBox(tr("Include additional information next to the state graph"));

	// Additional items selection group box

	this->additionalInfoSelectionGroup = new QGroupBox(tr("Additional information to include"));

	this->includeComponentCheckBox = new QCheckBox(tr("Include component external view"));
	this->includeInputsCheckBox    = new QCheckBox(tr("Include inputs"));
	this->includeOutputsCheckBox   = new QCheckBox(tr("Include outputs"));
	this->includeVariablesCheckBox = new QCheckBox(tr("Include internal variables"));
	this->includeConstantsCheckBox = new QCheckBox(tr("Include constants"));

	// Additional items configuration group box

	this->additionalInfoConfigurationGroup = new QGroupBox(tr("Configuration of the additional information"));

	this->addBorderCheckBox = new QCheckBox(tr("Display a border"));

	auto additionalItemsPositionLabel = new QLabel(tr("Position of the additional information:"));

	this->additionalInfoPositionComboBox = new QComboBox();
	this->additionalInfoPositionComboBox->addItem(tr("Right"));
	this->additionalInfoPositionComboBox->addItem(tr("Left"));
	this->additionalInfoPositionComboBox->setItemData(0, "RIGHT");
	this->additionalInfoPositionComboBox->setItemData(1, "LEFT");

	auto additionalInfoRatioSlider = new SliderWithTitleAndValue(1, 10, 3,
	                                                             tr("Ratio between state graph and additional information:"),
	                                                             "", ":1"
	                                                            );

	this->innerMarginSlider = new SliderWithTitleAndValue(0, 25, 3,
	                                                      tr("Additional margin inside each area:"),
	                                                      "", " %"
	                                                     );

	// Preview group box

	auto previewGroup = new QGroupBox(tr("Preview"));

	this->previewWidget = new QLabel();
	this->previewWidget->setMinimumSize(this->previewSidePixels, this->previewSidePixels);

	// Buttons

	auto buttonOK     = new QPushButton(tr("OK"));
	auto buttonCancel = new QPushButton(tr("Cancel"));

	//
	// Build layouts

	// Horizontal layout for image format combo box
	auto imageFormatLayout = new QHBoxLayout();
	imageFormatLayout->setContentsMargins(0, 0, 0, 0);
	imageFormatLayout->addWidget(imageFormatLabel,          0, Qt::AlignLeft);
	imageFormatLayout->addWidget(this->imageFormatComboBox, 0, Qt::AlignLeft);
	imageFormatLayout->addStretch(1);

	// Horizontal layout for additional items position combo box
	auto additionalItemsPositionLayout = new QHBoxLayout();
	additionalItemsPositionLayout->setContentsMargins(0, 0, 0, 0);
	additionalItemsPositionLayout->addWidget(additionalItemsPositionLabel,         0, Qt::AlignLeft);
	additionalItemsPositionLayout->addWidget(this->additionalInfoPositionComboBox, 0, Qt::AlignLeft);
	additionalItemsPositionLayout->addStretch(1);

	// Additional items selection group (level 2)
	auto additionalItemsSelectionLayout = new QVBoxLayout(this->additionalInfoSelectionGroup);
	additionalItemsSelectionLayout->addWidget(this->includeComponentCheckBox);
	additionalItemsSelectionLayout->addWidget(this->includeInputsCheckBox);
	additionalItemsSelectionLayout->addWidget(this->includeOutputsCheckBox);
	additionalItemsSelectionLayout->addWidget(this->includeVariablesCheckBox);
	additionalItemsSelectionLayout->addWidget(this->includeConstantsCheckBox);

	// Additional items configuration group (level 2)
	auto additionalInfoConfigurationLayout = new QVBoxLayout(this->additionalInfoConfigurationGroup);
	additionalInfoConfigurationLayout->addWidget(this->addBorderCheckBox);
	additionalInfoConfigurationLayout->addLayout(additionalItemsPositionLayout);
	additionalInfoConfigurationLayout->addWidget(additionalInfoRatioSlider);
	additionalInfoConfigurationLayout->addWidget(this->innerMarginSlider);

	// Main configuration group (level 1)
	auto mainConfigurationLayout = new QVBoxLayout(mainConfigurationGroup);
	mainConfigurationLayout->addLayout(imageFormatLayout);
	mainConfigurationLayout->addWidget(this->imageSizeEditor);
	mainConfigurationLayout->addWidget(outerMarginSlider);
	mainConfigurationLayout->addWidget(includeAdditionalInfoCheckBox);
	mainConfigurationLayout->addWidget(this->additionalInfoSelectionGroup);
	mainConfigurationLayout->addWidget(this->additionalInfoConfigurationGroup);

	// Preview group (level 1)
	auto previewLayout = new QVBoxLayout(previewGroup);
	previewLayout->addWidget(this->previewWidget, 0, Qt::AlignCenter);

	// Layout for level 1 groups
	auto globalHLayout = new QHBoxLayout();
	globalHLayout->addWidget(mainConfigurationGroup, 0, Qt::AlignTop);
	globalHLayout->addWidget(previewGroup, 0, Qt::AlignTop);

	// Horizontal layout for dialog buttons
	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->setContentsMargins(0, 0, 0, 0);
	buttonsLayout->addWidget(buttonOK);
	buttonsLayout->addWidget(buttonCancel);

	// Main layout: groups and buttons
	auto globalVLayout = new QVBoxLayout(this);
	globalVLayout->addLayout(globalHLayout);
	globalVLayout->addLayout(buttonsLayout);

	//
	// Set widgets initial visibility

	this->additionalInfoSelectionGroup    ->setVisible(false);
	this->additionalInfoConfigurationGroup->setVisible(false);

	//
	// Connect signals

	connect(includeAdditionalInfoCheckBox,  &QCheckBox::toggled, this, &ImageExportDialog::includeAdditionalInfoCheckBoxChanged);
	connect(this->includeComponentCheckBox, &QCheckBox::toggled, this, &ImageExportDialog::includeComponentCheckBoxChanged);
	connect(this->includeInputsCheckBox,    &QCheckBox::toggled, this, &ImageExportDialog::includeInputsCheckBoxChanged);
	connect(this->includeOutputsCheckBox,   &QCheckBox::toggled, this, &ImageExportDialog::includeOutputsCheckBoxChanged);
	connect(this->includeVariablesCheckBox, &QCheckBox::toggled, this, &ImageExportDialog::includeVariablesCheckBoxChanged);
	connect(this->includeConstantsCheckBox, &QCheckBox::toggled, this, &ImageExportDialog::includeConstantsCheckBoxChanged);
	connect(this->addBorderCheckBox,        &QCheckBox::toggled, this, &ImageExportDialog::addBorderCheckBoxChanged);

	connect(this->imageFormatComboBox,            &QComboBox::currentIndexChanged, this, &ImageExportDialog::imageFormatComboBoxChanged);
	connect(this->additionalInfoPositionComboBox, &QComboBox::currentIndexChanged, this, &ImageExportDialog::additionalInfoPositionComboBoxChanged);

	connect(additionalInfoRatioSlider, &SliderWithTitleAndValue::valueChangedEvent, this, &ImageExportDialog::additionalInfoRatioSliderChanged);
	connect(outerMarginSlider,         &SliderWithTitleAndValue::valueChangedEvent, this, &ImageExportDialog::outerMarginSliderChanged);
	connect(innerMarginSlider,         &SliderWithTitleAndValue::valueChangedEvent, this, &ImageExportDialog::innerMarginSliderChanged);

	connect(this->imageSizeEditor, &DocumentSizeEditor::selectedSizeChangedEvent, this, &ImageExportDialog::selectedSizeChangedEventHandler);

	connect(buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
	connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

	//
	// Update preview

	this->imageExporter->setVectorPageLayout(this->imageSizeEditor->getVectorPageLayout());
	this->updatePreview();
}

QString ImageExportDialog::getFilePath() const
{
	return this->outputFilePath;
}

shared_ptr<MachineImageExporter> ImageExportDialog::getImageExporter() const
{
	return this->imageExporter;
}

void ImageExportDialog::accept()
{
	auto imageFormat = this->getImageFormat();

	QString saveFilePath;
	switch (imageFormat)
	{
	case ImageFormat_t::pdf:
		saveFilePath = SaveFileDialog::getSaveFileName(this, tr("Export machine to Pdf"),  this->baseFilePath, this->baseFileName, "pdf");
		break;
	case ImageFormat_t::svg:
		saveFilePath = SaveFileDialog::getSaveFileName(this, tr("Export machine to Svg"),  this->baseFilePath, this->baseFileName, "svg");
		break;
	case ImageFormat_t::png:
		saveFilePath = SaveFileDialog::getSaveFileName(this, tr("Export machine to Png"),  this->baseFilePath, this->baseFileName, "png");
		break;
	case ImageFormat_t::jpg:
		saveFilePath = SaveFileDialog::getSaveFileName(this, tr("Export machine to Jpeg"), this->baseFilePath, this->baseFileName, "jpg");
		break;
	}

	if (saveFilePath.isEmpty() == false)
	{
		this->outputFilePath = saveFilePath;
		this->imageExporter->setImageFormat(imageFormat);

		QDialog::accept();
	}
}

void ImageExportDialog::includeAdditionalInfoCheckBoxChanged(bool doInclude)
{
	this->additionalInfoSelectionGroup    ->setVisible(doInclude);
	this->additionalInfoConfigurationGroup->setVisible(doInclude);

	if (doInclude == true)
	{
		if (this->haveAdditionalInfoBeenConfigured == false)
		{
			this->includeComponentCheckBox->setChecked(true);
			this->includeInputsCheckBox   ->setChecked(true);
			this->includeOutputsCheckBox  ->setChecked(true);
			this->includeVariablesCheckBox->setChecked(true);
			this->includeConstantsCheckBox->setChecked(true);
			this->addBorderCheckBox       ->setChecked(true);

			this->imageExporter->setInfoPosition(LeftRight_t::right);

			this->haveAdditionalInfoBeenConfigured = true;
		}
		else
		{
			this->imageExporter->setDisplayComponent(this->includeComponentCheckBox->isChecked());
			this->imageExporter->setDisplayInputs   (this->includeInputsCheckBox   ->isChecked());
			this->imageExporter->setDisplayOutputs  (this->includeOutputsCheckBox  ->isChecked());
			this->imageExporter->setDisplayConstants(this->includeConstantsCheckBox->isChecked());
			this->imageExporter->setDisplayVariables(this->includeVariablesCheckBox->isChecked());
			this->imageExporter->setDisplayBorder   (this->addBorderCheckBox       ->isChecked());
		}
	}
	else // (doInclude == false)
	{
		this->imageExporter->setDisplayComponent(false);
		this->imageExporter->setDisplayInputs   (false);
		this->imageExporter->setDisplayOutputs  (false);
		this->imageExporter->setDisplayConstants(false);
		this->imageExporter->setDisplayVariables(false);
		this->imageExporter->setDisplayBorder   (false);
	}

	this->updatePreview();
}

void ImageExportDialog::includeComponentCheckBoxChanged(bool checked)
{
	this->imageExporter->setDisplayComponent(checked);
	this->updatePreview();
}

void ImageExportDialog::includeInputsCheckBoxChanged(bool checked)
{
	this->imageExporter->setDisplayInputs(checked);
	this->updatePreview();
}

void ImageExportDialog::includeOutputsCheckBoxChanged(bool checked)
{
	this->imageExporter->setDisplayOutputs(checked);
	this->updatePreview();
}

void ImageExportDialog::includeVariablesCheckBoxChanged(bool checked)
{
	this->imageExporter->setDisplayVariables(checked);
	this->updatePreview();
}

void ImageExportDialog::includeConstantsCheckBoxChanged(bool checked)
{
	this->imageExporter->setDisplayConstants(checked);
	this->updatePreview();
}

void ImageExportDialog::addBorderCheckBoxChanged(bool checked)
{
	this->innerMarginSlider->setVisible(checked);

	this->imageExporter->setDisplayBorder(checked);
	this->updatePreview();
}

void ImageExportDialog::imageFormatComboBoxChanged(int)
{
	auto format = this->getImageFormat();

	switch (format)
	{
	case ImageFormat_t::pdf:
	case ImageFormat_t::svg:
		this->imageSizeEditor->setImageType(DocumentSizeEditor::ImageType_t::vector);
		break;
	case ImageFormat_t::png:
	case ImageFormat_t::jpg:
		this->imageSizeEditor->setImageType(DocumentSizeEditor::ImageType_t::bitmap);
		break;
	}

	this->imageExporter->setImageFormat(format);
	this->updatePreview();
}

void ImageExportDialog::additionalInfoPositionComboBoxChanged(int)
{
    if (this->additionalInfoPositionComboBox->currentData() == "LEFT")
	{
		this->imageExporter->setInfoPosition(LeftRight_t::left);
	}
	else // (this->additionalInfoPositionComboBox->currentData() == "RIGHT")
	{
		this->imageExporter->setInfoPosition(LeftRight_t::right);
	}
	this->updatePreview();
}

void ImageExportDialog::additionalInfoRatioSliderChanged(int ratio)
{
	this->imageExporter->setStateGraphRatio(ratio);
	this->updatePreview();
}

void ImageExportDialog::outerMarginSliderChanged(int margin)
{
	this->imageExporter->setOuterMargin(margin);
	this->updatePreview();
}

void ImageExportDialog::innerMarginSliderChanged(int margin)
{
	this->imageExporter->setInnerMargin(margin);
	this->updatePreview();
}

void ImageExportDialog::selectedSizeChangedEventHandler()
{
	auto format = this->getImageFormat();

	switch (format)
	{
	case ImageFormat_t::pdf:
	case ImageFormat_t::svg:
	{
		auto pageLayout = this->imageSizeEditor->getVectorPageLayout();
		if (pageLayout.isValid() == false) return;


		this->imageExporter->setVectorPageLayout(pageLayout);
		this->updatePreview();

		break;
	}
	case ImageFormat_t::png:
	case ImageFormat_t::jpg:
	{
		auto bitmapSize = this->imageSizeEditor->getBitmapSize();
		if (bitmapSize.isNull() == true) return;


		this->imageExporter->setBitmapSize(bitmapSize);
		this->updatePreview();

		break;
	}
	}
}

ImageFormat_t ImageExportDialog::getImageFormat() const
{
	return static_cast<ImageFormat_t>(this->imageFormatComboBox->currentData().toInt());
}

void ImageExportDialog::updatePreview()
{
	auto previewPixmap = this->imageExporter->renderPreview(this->previewSidePixels);
	this->previewWidget->setPixmap(*previewPixmap);
}
