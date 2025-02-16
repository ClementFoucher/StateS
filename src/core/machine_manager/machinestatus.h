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

#ifndef MACHINESTATUS_H
#define MACHINESTATUS_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QFileInfo>


/**
 * @brief The MachineStatus class strores meta-information required
 * during the application life, such as save status and pathes.
 * Its mutators must only be used by the StateS class (including
 * its undo-redo manager), with the exception of some pathes that
 * are only used by StatesUI, thus managed by it.
 */
class MachineStatus : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineStatus();

	/////
	// Object functions
public:
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

	/////
	// Signals
signals:
	void saveFilePathChangedEvent();
	void unsavedFlagChangedEvent();

	/////
	// Object variables
private:
	QFileInfo saveFilePath;
	QFileInfo imageExportPath;
	QFileInfo vhdlExportPath;
	bool      unsavedFlag;
	bool      hasSaveFile;
};

#endif // MACHINESTATUS_H
