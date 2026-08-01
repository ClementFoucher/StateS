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

#ifndef DOCUMENTSIZEEDITOR_H
#define DOCUMENTSIZEEDITOR_H

// Parent
#include <QWidget>

// Qt
#include <QPageLayout>
class QComboBox;
class QLabel;

// StateS
class FixedSizeLineEditor;


class DocumentSizeEditor : public QWidget
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class ImageType_t
	{
		uninitialized,
		vector,
		bitmap
	};

	/////
	// Constructors/destructors
public:
	explicit DocumentSizeEditor(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setImageType(ImageType_t type);

	QPageLayout getVectorPageLayout() const;
	QSize getBitmapSize() const;

private slots:
	void imageSizeComboBoxSelectedIndexChangedEventHandler(int);
	void imageOrientationComboBoxSelectedIndexChangedEventHandler(int);
	void imageSizeUnitComboBoxSelectedIndexChangedEventHandler(int);
	void sizeLineEditChangedEventHandler(const QString&);

private:
	void fillImageSizeComboBox();
	void applyCustomSize();
	void displayCurrentSizeInSizeTextFields();
	void setPageLayout(const QPageSize& pageSize, QPageLayout::Orientation orientation, QPageLayout::Unit unit);

	/////
	// Signals
signals:
	void selectedSizeChangedEvent();

	/////
	// Object variables
private:
	// 5000 pixels is a sensible default that will output
	// a good quality image for medium-size machines
	const uint defaultSideForBitmapImages = 5000;

	ImageType_t imageType = ImageType_t::uninitialized;

	QPageLayout vectorPageLayout;
	QSize       bitmapSize;

	QWidget* imageOrientationWidget = nullptr;

	QComboBox* imageOrientationComboBox = nullptr;
	QComboBox* imageSizeComboBox      = nullptr;
	QComboBox* imageSizeUnitComboBox  = nullptr;

	FixedSizeLineEditor* imageSizeWidthLineEdit  = nullptr;
	FixedSizeLineEditor* imageSizeHeightLineEdit = nullptr;

	QLabel* pixelsLabel = nullptr;

};

#endif // DOCUMENTSIZEEDITOR_H
