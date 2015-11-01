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

#ifndef IMAGEEXPORTDIALOG_H
#define IMAGEEXPORTDIALOG_H

// Parent
#include <QDialog>

// Qt classes
class QComboBox;

// StateS classes
#include "machineimageexporter.h"
class CheckBoxHtml;


class ImageExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageExportDialog(const QString& baseFileName, const QString& searchPath = QString::null, QWidget* parent = nullptr);

    bool includeComponent();
    MachineImageExporter::imageFormat getImageFormat();
    QString getFilePath();

protected:
    void accept() override;

private:
    QComboBox*    imageFormatSelectionBox  = nullptr;
    CheckBoxHtml* includeComponentCheckBox = nullptr;

    QString baseFileName;
    QString searchPath;
    QString filePath = QString::null;
};

#endif // IMAGEEXPORTDIALOG_H
