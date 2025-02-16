/*
 * Copyright © 2020-2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "machinestatus.h"


/////
// Constructors/destructors

MachineStatus::MachineStatus()
{
	this->unsavedFlag     = false;
	this->hasSaveFile     = false;
	this->saveFilePath    = QFileInfo();
	this->imageExportPath = QFileInfo();
	this->vhdlExportPath  = QFileInfo();
}

/////
// Mutators

void MachineStatus::setUnsavedFlag(bool newUnsavedFlag)
{
	if (newUnsavedFlag != this->unsavedFlag)
	{
		this->unsavedFlag = newUnsavedFlag;
		emit unsavedFlagChangedEvent();
	}
}

void MachineStatus::setHasSaveFile(bool newHasSaveFile)
{
	this->hasSaveFile = newHasSaveFile;
}

void MachineStatus::setSaveFilePath(const QString& newPath)
{
	QFileInfo newFile(newPath);
	if (newFile != this->saveFilePath)
	{
		this->saveFilePath = newFile;
		emit saveFilePathChangedEvent();
	}
}

void MachineStatus::setImageExportPath(const QString& newPath)
{
	this->imageExportPath = QFileInfo(newPath);
}

void MachineStatus::setVhdlExportPath(const QString& newPath)
{
	this->vhdlExportPath = QFileInfo(newPath);
}

/////
// Accessors

bool MachineStatus::getUnsavedFlag() const
{
	return this->unsavedFlag;
}

bool MachineStatus::getHasSaveFile() const
{
	return this->hasSaveFile;
}

QString MachineStatus::getSaveFilePath() const
{
	return this->saveFilePath.path();
}

QString MachineStatus::getSaveFileFullPath() const
{
	return this->saveFilePath.filePath();
}

QString MachineStatus::getImageExportPath() const
{
	if (this->imageExportPath.path().isEmpty() == false)
	{
		return this->imageExportPath.path();
	}
	else
	{
		return this->saveFilePath.path();
	}
}

QString MachineStatus::getVhdlExportPath() const
{
	if (this->vhdlExportPath.path().isEmpty() == false)
	{
		return this->vhdlExportPath.path();
	}
	else
	{
		return this->saveFilePath.path();
	}
}
