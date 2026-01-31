/*
 * Copyright © 2020-2026 Clément Foucher
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

// StateS classes
#include "states.h"


/////
// Constructors/destructors

MachineStatus::MachineStatus()
{
	if (StateS::hasSetting("SaveLoadPath") == true)
	{
		this->saveLoadFolderPath = QFileInfo(StateS::retreiveSetting("SaveLoadPath").toString());
	}

	if (StateS::hasSetting("ImageExportPath") == true)
	{
		this->imageExportFolderPath = QFileInfo(StateS::retreiveSetting("ImageExportPath").toString());
	}

	if (StateS::hasSetting("VhdlExportPath") == true)
	{
		this->vhdlExportFolderPath = QFileInfo(StateS::retreiveSetting("VhdlExportPath").toString());
	}
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
	if (newPath == this->saveFilePath.path()) return;


	this->saveFilePath = QFileInfo(newPath);
	this->setSaveLoadFolderPath(this->saveFilePath.absolutePath());

	emit this->saveFilePathChangedEvent();
}

void MachineStatus::setSaveLoadFolderPath(const QString& newPath)
{
	this->saveLoadFolderPath = QFileInfo(newPath);

	StateS::storeSetting("SaveLoadPath", this->saveLoadFolderPath.absoluteFilePath());
}

void MachineStatus::setImageExportFolderPath(const QString& newPath)
{
	this->imageExportFolderPath = QFileInfo(newPath);

	StateS::storeSetting("ImageExportPath", this->imageExportFolderPath.absoluteFilePath());
}

void MachineStatus::setVhdlExportFolderPath(const QString& newPath)
{
	this->vhdlExportFolderPath = QFileInfo(newPath);

	StateS::storeSetting("VhdlExportPath", this->vhdlExportFolderPath.absoluteFilePath());
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
	return this->saveFilePath.absoluteFilePath();
}

QString MachineStatus::getSaveLoadFolderPath() const
{
	return this->saveLoadFolderPath.absoluteFilePath();
}

QString MachineStatus::getImageExportFolderPath() const
{
	if (this->imageExportFolderPath.path().isEmpty() == false)
	{
		return this->imageExportFolderPath.absoluteFilePath();
	}
	else
	{
		return this->saveFilePath.path();
	}
}

QString MachineStatus::getVhdlExportFolderPath() const
{
	if (this->vhdlExportFolderPath.path().isEmpty() == false)
	{
		return this->vhdlExportFolderPath.absoluteFilePath();
	}
	else
	{
		return this->saveFilePath.path();
	}
}
