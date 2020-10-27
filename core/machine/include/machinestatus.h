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

// Qt classes
#include <QFileInfo>

// C++ classes
#include <memory>
using namespace std;


class MachineStatus : public QObject
{
	Q_OBJECT

public:
	explicit MachineStatus();
	explicit MachineStatus(shared_ptr<MachineStatus> fromObject);

	static shared_ptr<MachineStatus> clonePaths(shared_ptr<MachineStatus> fromObject);

	void setUnsavedFlag    (bool newUnsavedFlag);
	void setHasSaveFile    (bool newHasSaveFile);
	void setSaveFilePath   (const QString& newPath);
	void setImageExportPath(const QString& newPath);
	void setVhdlExportPath (const QString& newPath);

	bool    getUnsavedFlag()      const;
	bool    getHasSaveFile()      const;
	QString getSaveFilePath()     const;
	QString getSaveFileFullPath() const;
	QString getImageExportPath()  const;
	QString getVhdlExportPath()   const;

signals:
	void saveFilePathChanged();
	void unsavedFlagChanged();

private:
	QFileInfo saveFilePath;
	QFileInfo imageExportPath;
	QFileInfo vhdlExportPath;
	bool      unsavedFlag;
	bool      hasSaveFile;
};

#endif // MACHINESTATUS_H
