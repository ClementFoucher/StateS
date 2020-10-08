/*
 * Copyright © 2014-2020 Clément Foucher
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
#include <QDialog>

// Qt classes
class QComboBox;
class QLabel;
class QSlider;

// StateS classes
#include "machineimageexporter.h"
class CheckBoxHtml;


class ImageExportDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ImageExportDialog(const QString& baseFileName, shared_ptr<MachineImageExporter> imageExporter, const QString& searchPath = QString(), QWidget* parent = nullptr);

	MachineImageExporter::imageFormat getImageFormat();
	QString getFilePath();

protected:
	virtual void accept() override;
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
	QString filePath = QString();
};

#endif // IMAGEEXPORTDIALOG_H
