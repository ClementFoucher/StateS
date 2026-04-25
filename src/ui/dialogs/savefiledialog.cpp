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
#include "savefiledialog.h"

// Qt classes
#include <QFileDialog>
#include <QMessageBox>


QString SaveFileDialog::getSaveFileName(QWidget* parent, const QString& caption, const QString& filePath, const QString& fileName, const QString& fileExtension)
{
	auto saveFilePath = QFileDialog::getSaveFileName(parent, caption, filePath + "/" + fileName + "." + fileExtension, "*." + fileExtension);
	if (saveFilePath.isEmpty() == true) return QString();


	if (saveFilePath.endsWith("." + fileExtension, Qt::CaseInsensitive) == false)
	{
		saveFilePath += "." + fileExtension;

		QFileInfo file(saveFilePath);
		if (file.isDir())
		{
			QString message = file.absoluteFilePath() + " " + tr("already exists and is a folder.");
			message += " " + tr("StateS is unable to write a file with this name.");
			QMessageBox::warning(parent, tr("Unable to overwrite"), message);

			return QString();
		}
		else if (file.isFile())
		{
			QString message = tr("File") + " " + file.absoluteFilePath() + " " + tr("already exists.");
			message += "<br>";
			message += tr("Do you want to overwrite it?");
			auto reply = QMessageBox::question(parent, tr("User confirmation needed"), message, QMessageBox::Ok | QMessageBox::Cancel);

			if (reply != QMessageBox::Ok) return QString();
		}
	}

	return saveFilePath;
}
