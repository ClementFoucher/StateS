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

#ifndef ERRORDISPLAYDIALOG_H
#define ERRORDISPLAYDIALOG_H

// Parent
#include "statesdialog.h"


class ErrorDisplayDialog : public StatesDialog
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ErrorDisplayDialog(const QString& errorTitle, const QList<QString>& errors, QWidget* parent = nullptr);
	explicit ErrorDisplayDialog(const QString& errorTitle, const QString& error, QWidget* parent = nullptr);

	/////
	// Object functions
private:
	void build(const QString& errorTitle, const QList<QString>& errors);

};

#endif // ERRORDISPLAYDIALOG_H
