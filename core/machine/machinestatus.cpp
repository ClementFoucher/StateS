/*
 * Copyright © 2020 Clément Foucher
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

#include "machinestatus.h"

MachineStatus::MachineStatus(const QString& filePath, QObject* parent) :
    QObject(parent)
{
	this->currentFilePath = filePath;
	this->unsavedFlag     = false;
}

void MachineStatus::setCurrentFilePath(const QString& newFilePath)
{
	if (newFilePath != this->currentFilePath)
	{
		this->currentFilePath = newFilePath;
		emit currentFilePathChanged();
	}
}

void MachineStatus::setUnsavedFlag(bool newUnsavedFlag)
{
	if (newUnsavedFlag != this->unsavedFlag)
	{
		this->unsavedFlag = newUnsavedFlag;
		emit unsavedFlagChanged();
	}
}

QString MachineStatus::getCurrentFilePath() const
{
	return this->currentFilePath;
}

bool MachineStatus::getUnsavedFlag() const
{
	return this->unsavedFlag;
}
