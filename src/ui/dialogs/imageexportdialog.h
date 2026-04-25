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

#ifndef IMAGEEXPORTDIALOG_H
#define IMAGEEXPORTDIALOG_H

// Parent
#include "statesdialog.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QComboBox;
class QLabel;
class QCheckBox;
class QGroupBox;

// StateS classes
#include "statestypes.h"
class MachineImageExporter;
class DocumentSizeEditor;
class SliderWithTitleAndValue;
class StatesScene;


class ImageExportDialog : public StatesDialog
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ImageExportDialog(StatesScene* stateGraphScene, shared_ptr<StatesScene> componentScene, const QString& baseFilePath, const QString& baseFileName, QWidget* parent = nullptr);

	/////
	// Object functions
public slots:
	virtual void accept() override;

public:
	QString getFilePath() const;

	shared_ptr<MachineImageExporter> getImageExporter() const;

private slots:
	void includeAdditionalInfoCheckBoxChanged(bool doInclude);
	void includeComponentCheckBoxChanged(bool checked);
	void includeInputsCheckBoxChanged(bool checked);
	void includeOutputsCheckBoxChanged(bool checked);
	void includeVariablesCheckBoxChanged(bool checked);
	void includeConstantsCheckBoxChanged(bool checked);
	void addBorderCheckBoxChanged(bool checked);
	void imageFormatComboBoxChanged(int);
	void additionalInfoPositionComboBoxChanged(int);
	void additionalInfoRatioSliderChanged(int ratio);
	void outerMarginSliderChanged(int margin);
	void innerMarginSliderChanged(int margin);
	void selectedSizeChangedEventHandler();

private:
	ImageFormat_t getImageFormat() const;
	void updatePreview();

	/////
	// Object variables
private:
	// Constant
	const uint previewSidePixels = 500;

	// Widgets
	QGroupBox* additionalInfoSelectionGroup     = nullptr;
	QGroupBox* additionalInfoConfigurationGroup = nullptr;

	QComboBox* imageFormatComboBox            = nullptr;
	QComboBox* additionalInfoPositionComboBox = nullptr;

	QCheckBox* addBorderCheckBox        = nullptr;
	QCheckBox* includeComponentCheckBox = nullptr;
	QCheckBox* includeInputsCheckBox    = nullptr;
	QCheckBox* includeOutputsCheckBox   = nullptr;
	QCheckBox* includeVariablesCheckBox = nullptr;
	QCheckBox* includeConstantsCheckBox = nullptr;

	DocumentSizeEditor* imageSizeEditor = nullptr;

	SliderWithTitleAndValue* innerMarginSlider = nullptr;

	QLabel* previewWidget = nullptr;

	// Other objects
	shared_ptr<MachineImageExporter> imageExporter;

	// Paths
	QString baseFilePath;
	QString baseFileName;
	QString outputFilePath;

	// Status
	bool haveAdditionalInfoBeenConfigured = false;

};

#endif // IMAGEEXPORTDIALOG_H
