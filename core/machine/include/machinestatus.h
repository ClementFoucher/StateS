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

#ifndef MACHINESTATUS_H
#define MACHINESTATUS_H

// Parent class
#include <QObject>

class MachineStatus : public QObject
{
	Q_OBJECT

public:
	explicit MachineStatus(const QString& filePath = QString(), QObject* parent = nullptr);

	void setCurrentFilePath(const QString& newFilePath);
	void setUnsavedFlag    (bool newUnsavedFlag);

	QString getCurrentFilePath() const;
	bool    getUnsavedFlag()     const;

signals:
	void currentFilePathChanged();
	void unsavedFlagChanged();

private:
	QString currentFilePath;
	bool    unsavedFlag;
};

#endif // MACHINESTATUS_H
