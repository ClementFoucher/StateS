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
class QSlider;

// StateS classes
#include "statestypes.h"
class MachineImageExporter;
class CheckBoxHtml;


class ImageExportDialog : public StatesDialog
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ImageExportDialog(const QString& baseFileName, shared_ptr<MachineImageExporter> imageExporter, const QString& searchPath, QWidget* parent = nullptr);

	/////
	// Object functions
public slots:
	virtual void accept() override;

public:
	ImageFormat_t getImageFormat() const;
	QString getFilePath() const;

	shared_ptr<MachineImageExporter> getImageExporter() const;

protected slots:
	virtual void resizeEvent(QResizeEvent*) override;

private slots:
	void includeComponentCheckBoxChanged(bool b);
	void includeConstantsCheckBoxChanged(bool b);
	void includeVariablesCheckBoxChanged(bool b);
	void infoToTheRightCheckBoxChanged(bool b);
	void addBorderCheckBoxChanged(bool b);
	void ratioSliderValueChanged(int i);

private:
	void updatePreview();

	/////
	// Object variables
private:
	QComboBox*    imageFormatSelectionBox  = nullptr;
	CheckBoxHtml* includeComponentCheckBox = nullptr;
	CheckBoxHtml* includeConstantsCheckBox = nullptr;
	CheckBoxHtml* includeVariablesCheckBox = nullptr;
	CheckBoxHtml* infoToTheRightCheckBox   = nullptr;
	CheckBoxHtml* addBorderCheckBox        = nullptr;
	QSlider*      ratioSlider              = nullptr;
	QLabel*       previewWidget            = nullptr;

	shared_ptr<MachineImageExporter> previewManager;
	shared_ptr<QPixmap> previewPixmap;

	QString baseFileName;
	QString searchPath;
	QString filePath;

};

#endif // IMAGEEXPORTDIALOG_H
