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

#ifndef VHDLEXPORTDIALOG_H
#define VHDLEXPORTDIALOG_H

// Parent
#include "statesdialog.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QComboBox;

// StateS classes
class FsmVhdlExport;


class VhdlExportDialog : public StatesDialog
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit VhdlExportDialog(const QString& baseFileName, const QString& searchPath, shared_ptr<FsmVhdlExport> fsmVhdlExport, QWidget* parent = nullptr);

	/////
	// Object functions
public slots:
	virtual void accept() override;

public:
	bool isResetPositive() const;
	bool prefixIOs() const;
	QString getFilePath() const;

	shared_ptr<FsmVhdlExport> getFsmVhdlExport() const;

	/////
	// Object variables
private:
	QComboBox* resetLogicSelectionBox = nullptr;
	QComboBox* addPrefixSelectionBox  = nullptr;

	QString baseFileName;
	QString searchPath;
	QString filePath;

	shared_ptr<FsmVhdlExport> fsmVhdlExport;

};

#endif // VHDLEXPORTDIALOG_H
