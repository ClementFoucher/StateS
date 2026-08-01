/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "documentsizeeditor.h"

// Qt
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>

// StateS
#include "fixedsizelineeditor.h"


DocumentSizeEditor::DocumentSizeEditor(QWidget* parent) :
	QWidget(parent)
{
	//
	// Build widgets

	// Image size
	auto imageSizeLabel = new QLabel(tr("Size:"));

	this->imageSizeComboBox = new QComboBox();

	this->imageSizeWidthLineEdit  = new FixedSizeLineEditor(5);
	auto imageSizeTimesSymbol     = new QLabel("×");
	this->imageSizeHeightLineEdit = new FixedSizeLineEditor(5);

	this->imageSizeWidthLineEdit ->setValidator(new QIntValidator(0, 99999));
	this->imageSizeHeightLineEdit->setValidator(new QIntValidator(0, 99999));

	this->imageSizeUnitComboBox = new QComboBox();

	this->imageSizeUnitComboBox->addItem(tr("mm"));
	this->imageSizeUnitComboBox->addItem(tr("in"));

	this->imageSizeUnitComboBox->setItemData(0, QPageSize::Unit::Millimeter);
	this->imageSizeUnitComboBox->setItemData(1, QPageSize::Unit::Inch);

	this->pixelsLabel = new QLabel(tr("pixels"));

	// Image orientation
	auto imageOrientationLabel = new QLabel(tr("Orientation:"));

	this->imageOrientationComboBox = new QComboBox();
	this->imageOrientationComboBox->addItem(tr("Landscape"));
	this->imageOrientationComboBox->addItem(tr("Portrait"));
	this->imageOrientationComboBox->setItemData(0, QPageLayout::Orientation::Landscape);
	this->imageOrientationComboBox->setItemData(1, QPageLayout::Orientation::Portrait);

	this->imageOrientationWidget = new QWidget();

	//
	//  Build layouts

	// Image size layout
	auto sizeLayout = new QHBoxLayout();
	sizeLayout->setContentsMargins(0, 0, 0, 0);
	sizeLayout->addWidget(imageSizeLabel,                0, Qt::AlignLeft);
	sizeLayout->addWidget(this->imageSizeComboBox,       0, Qt::AlignLeft);
	sizeLayout->addWidget(this->imageSizeWidthLineEdit,  0, Qt::AlignLeft);
	sizeLayout->addWidget(imageSizeTimesSymbol,          0, Qt::AlignLeft);
	sizeLayout->addWidget(this->imageSizeHeightLineEdit, 0, Qt::AlignLeft);
	sizeLayout->addWidget(this->imageSizeUnitComboBox,   0, Qt::AlignLeft);
	sizeLayout->addWidget(this->pixelsLabel,             0, Qt::AlignLeft);
	sizeLayout->addStretch(1);

	// Image direction widget layout
	auto imageOrientationLayout = new QHBoxLayout(this->imageOrientationWidget);
	imageOrientationLayout->setContentsMargins(0, 0, 0, 0);
	imageOrientationLayout->addWidget(imageOrientationLabel,          0, Qt::AlignLeft);
	imageOrientationLayout->addWidget(this->imageOrientationComboBox, 0, Qt::AlignLeft);
	imageOrientationLayout->addStretch(1);

	// Main layout
	auto globalLayout = new QVBoxLayout(this);
	globalLayout->setContentsMargins(0, 0, 0, 0);
	globalLayout->addLayout(sizeLayout);
	globalLayout->addWidget(this->imageOrientationWidget);

	//
	// Connect signals

	connect(this->imageSizeComboBox,        &QComboBox::currentIndexChanged, this, &DocumentSizeEditor::imageSizeComboBoxSelectedIndexChangedEventHandler);
	connect(this->imageSizeUnitComboBox,    &QComboBox::currentIndexChanged, this, &DocumentSizeEditor::imageSizeUnitComboBoxSelectedIndexChangedEventHandler);
	connect(this->imageOrientationComboBox, &QComboBox::currentIndexChanged, this, &DocumentSizeEditor::imageOrientationComboBoxSelectedIndexChangedEventHandler);

	connect(this->imageSizeWidthLineEdit,  &QLineEdit::textChanged, this, &DocumentSizeEditor::sizeLineEditChangedEventHandler);
	connect(this->imageSizeHeightLineEdit, &QLineEdit::textChanged, this, &DocumentSizeEditor::sizeLineEditChangedEventHandler);

	//
	// Set object members

	this->setImageType(ImageType_t::vector);
	this->setPageLayout(QPageSize(QPageSize::PageSizeId::A4), QPageLayout::Orientation::Landscape, QPageLayout::Unit::Millimeter);
}

void DocumentSizeEditor::setImageType(ImageType_t type)
{
	if (this->imageType == type) return;

	if (type == ImageType_t::uninitialized) return;


	this->imageType = type;

	this->fillImageSizeComboBox();

	switch (this->imageType)
	{
	case ImageType_t::vector:
		this->imageSizeUnitComboBox ->setVisible(true);
		this->pixelsLabel           ->setVisible(false);
		this->imageOrientationWidget->setVisible(true);
		break;
	case ImageType_t::bitmap:
		this->imageSizeUnitComboBox ->setVisible(false);
		this->pixelsLabel           ->setVisible(true);
		this->imageOrientationWidget->setVisible(false);
		break;
	case ImageType_t::uninitialized:
		// XKCD 2200
		break;
	}
}

QPageLayout DocumentSizeEditor::getVectorPageLayout() const
{
	if (this->imageType != ImageType_t::vector) return QPageLayout();


	return this->vectorPageLayout;
}

QSize DocumentSizeEditor::getBitmapSize() const
{
	if (this->imageType != ImageType_t::bitmap) return QSize();


	return this->bitmapSize;
}

void DocumentSizeEditor::imageSizeComboBoxSelectedIndexChangedEventHandler(int)
{
	auto imageSizeCurrentData = this->imageSizeComboBox->currentData().toInt();
	auto selectedSize = static_cast<QPageSize::PageSizeId>(imageSizeCurrentData);

	// Update widgets state
	if (selectedSize == QPageSize::PageSizeId::Custom)
	{
		this->imageSizeWidthLineEdit ->setEnabled(true);
		this->imageSizeHeightLineEdit->setEnabled(true);

		if (this->imageType ==ImageType_t::vector)
		{
			this->imageSizeUnitComboBox ->setEnabled(true);
			this->imageOrientationWidget->setVisible(false);
		}
	}
	else
	{
		this->imageSizeWidthLineEdit ->setEnabled(false);
		this->imageSizeHeightLineEdit->setEnabled(false);

		if (this->imageType ==ImageType_t::vector)
		{
			this->imageSizeUnitComboBox ->setEnabled(false);
			this->imageOrientationWidget->setVisible(true);
		}
	}

	// Compute new value
	// Do not change line edits value for custom size: stay with the latest selected size
	if (selectedSize != QPageSize::PageSizeId::Custom)
	{
		switch (this->imageType)
		{
		case ImageType_t::vector:
		{
			auto pageOrientation = static_cast<QPageLayout::Orientation>(this->imageOrientationComboBox->currentData().toInt());

			if (selectedSize == QPageSize::PageSizeId::Letter)
			{
				this->setPageLayout(QPageSize(selectedSize), pageOrientation, QPageLayout::Unit::Inch);
			}
			else
			{
				this->setPageLayout(QPageSize(selectedSize), pageOrientation, QPageLayout::Unit::Millimeter);
			}
		}
		break;
		case ImageType_t::bitmap:
			// Only auto mode is possible here
			this->bitmapSize = QSize(this->defaultSideForBitmapImages, this->defaultSideForBitmapImages);

			break;
		case ImageType_t::uninitialized:
			// XKCD 2200
			break;
		}

		this->displayCurrentSizeInSizeTextFields();

		emit this->selectedSizeChangedEvent();
	}
}

void DocumentSizeEditor::imageOrientationComboBoxSelectedIndexChangedEventHandler(int)
{
	auto pageOrientation = static_cast<QPageLayout::Orientation>(this->imageOrientationComboBox->currentData().toInt());
	this->vectorPageLayout.setOrientation(pageOrientation);

	this->displayCurrentSizeInSizeTextFields();

	emit this->selectedSizeChangedEvent();
}

void DocumentSizeEditor::imageSizeUnitComboBoxSelectedIndexChangedEventHandler(int)
{
	this->applyCustomSize();
}

void DocumentSizeEditor::sizeLineEditChangedEventHandler(const QString&)
{
	this->applyCustomSize();
}

void DocumentSizeEditor::fillImageSizeComboBox()
{
	disconnect(this->imageSizeComboBox, &QComboBox::currentIndexChanged, this, &DocumentSizeEditor::imageSizeComboBoxSelectedIndexChangedEventHandler);

	this->imageSizeComboBox->clear();

	switch (this->imageType)
	{
	case ImageType_t::vector:
		this->imageSizeComboBox->addItem(tr("Custom"));
		this->imageSizeComboBox->addItem("A0");
		this->imageSizeComboBox->addItem("A1");
		this->imageSizeComboBox->addItem("A2");
		this->imageSizeComboBox->addItem("A3");
		this->imageSizeComboBox->addItem("A4");
		this->imageSizeComboBox->addItem("A5");
		this->imageSizeComboBox->addItem("A6");
		this->imageSizeComboBox->addItem("A7");
		this->imageSizeComboBox->addItem("US Letter");

		this->imageSizeComboBox->setItemData(0, QPageSize::PageSizeId::Custom);
		this->imageSizeComboBox->setItemData(1, QPageSize::PageSizeId::A0);
		this->imageSizeComboBox->setItemData(2, QPageSize::PageSizeId::A1);
		this->imageSizeComboBox->setItemData(3, QPageSize::PageSizeId::A2);
		this->imageSizeComboBox->setItemData(4, QPageSize::PageSizeId::A3);
		this->imageSizeComboBox->setItemData(5, QPageSize::PageSizeId::A4);
		this->imageSizeComboBox->setItemData(6, QPageSize::PageSizeId::A5);
		this->imageSizeComboBox->setItemData(7, QPageSize::PageSizeId::A6);
		this->imageSizeComboBox->setItemData(8, QPageSize::PageSizeId::A7);
		this->imageSizeComboBox->setItemData(9, QPageSize::PageSizeId::Letter);

		this->imageSizeComboBox->setCurrentIndex(5);

		break;
	case ImageType_t::bitmap:
		this->imageSizeComboBox->addItem("Auto");
		this->imageSizeComboBox->addItem(tr("Custom"));

		this->imageSizeComboBox->setItemData(0, -1);
		this->imageSizeComboBox->setItemData(1, QPageSize::PageSizeId::Custom);

		this->imageSizeComboBox->setCurrentIndex(0);

		break;
	case ImageType_t::uninitialized:
		// XKCD 2200
		break;
	}

	this->imageSizeComboBoxSelectedIndexChangedEventHandler(this->imageSizeComboBox->currentIndex());

	connect(this->imageSizeComboBox, &QComboBox::currentIndexChanged, this, &DocumentSizeEditor::imageSizeComboBoxSelectedIndexChangedEventHandler);
}

void DocumentSizeEditor::applyCustomSize()
{
	auto imageSizeCurrentData = this->imageSizeComboBox->currentData().toInt();
	auto selectedSize = static_cast<QPageSize::PageSizeId>(imageSizeCurrentData);

	if (selectedSize != QPageSize::PageSizeId::Custom) return;

	qreal width  = this->imageSizeWidthLineEdit ->text().toDouble();
	qreal height = this->imageSizeHeightLineEdit->text().toDouble();

	if ( (width <= 0) || (height <= 0) ) return;


	switch (this->imageType)
	{
	case ImageType_t::vector:
	{
		int imageSizeUnit = this->imageSizeUnitComboBox->currentData().toInt();
		auto pageSizeUnit   = static_cast<QPageSize::Unit>(imageSizeUnit);
		auto pageLayoutUnit = static_cast<QPageLayout::Unit>(imageSizeUnit);

		if (height >= width)
		{
			QPageSize newPageSize(QSize(width, height), pageSizeUnit);
			this->setPageLayout(newPageSize, QPageLayout::Orientation::Portrait, pageLayoutUnit);
		}
		else // width > height
		{
			QPageSize newPageSize(QSize(height, width), pageSizeUnit);
			this->setPageLayout(newPageSize, QPageLayout::Orientation::Landscape, pageLayoutUnit);
		}

		if (this->vectorPageLayout.isValid() == true)
		{
			emit this->selectedSizeChangedEvent();
		}

		break;
	}
	case ImageType_t::bitmap:
		this->bitmapSize = QSize(width, height);

		emit this->selectedSizeChangedEvent();

		break;
	case ImageType_t::uninitialized:
		// XKCD 2200
		break;
	}
}

void DocumentSizeEditor::displayCurrentSizeInSizeTextFields()
{
	disconnect(this->imageSizeWidthLineEdit,  &QLineEdit::textChanged,         this, &DocumentSizeEditor::sizeLineEditChangedEventHandler);
	disconnect(this->imageSizeHeightLineEdit, &QLineEdit::textChanged,         this, &DocumentSizeEditor::sizeLineEditChangedEventHandler);
	disconnect(this->imageSizeUnitComboBox,   &QComboBox::currentIndexChanged, this, &DocumentSizeEditor::imageSizeUnitComboBoxSelectedIndexChangedEventHandler);

	switch (this->imageType)
	{
	case ImageType_t::vector:
	{
		auto pageLayoutRect = this->vectorPageLayout.fullRect();
		this->imageSizeWidthLineEdit ->setText(QString::number(pageLayoutRect.width()));
		this->imageSizeHeightLineEdit->setText(QString::number(pageLayoutRect.height()));

		if (this->vectorPageLayout.units() == QPageLayout::Unit::Millimeter)
		{
			this->imageSizeUnitComboBox->setCurrentText(tr("mm"));
		}
		else // Inch
		{
			this->imageSizeUnitComboBox->setCurrentText(tr("in"));
		}

		break;
	}
	case ImageType_t::bitmap:
		this->imageSizeWidthLineEdit ->setText(QString::number(this->bitmapSize.width()));
		this->imageSizeHeightLineEdit->setText(QString::number(this->bitmapSize.height()));

		break;
	case ImageType_t::uninitialized:
		// XKCD 2200
		break;
	}

	connect(this->imageSizeWidthLineEdit,  &QLineEdit::textChanged,         this, &DocumentSizeEditor::sizeLineEditChangedEventHandler);
	connect(this->imageSizeHeightLineEdit, &QLineEdit::textChanged,         this, &DocumentSizeEditor::sizeLineEditChangedEventHandler);
	connect(this->imageSizeUnitComboBox,   &QComboBox::currentIndexChanged, this, &DocumentSizeEditor::imageSizeUnitComboBoxSelectedIndexChangedEventHandler);
}

void DocumentSizeEditor::setPageLayout(const QPageSize& pageSize, QPageLayout::Orientation orientation, QPageLayout::Unit unit)
{
	this->vectorPageLayout = QPageLayout(pageSize, orientation, QMarginsF(), unit);
	this->vectorPageLayout.setMode(QPageLayout::Mode::FullPageMode);
}
