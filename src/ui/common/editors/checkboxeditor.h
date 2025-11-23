/*
 * Copyright © 2025 Clément Foucher
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

#ifndef CHECKBOXEDITOR_H
#define CHECKBOXEDITOR_H

// Parent
#include <QWidget>

// Qt classes
class QCheckBox;


class CheckBoxEditor : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit CheckBoxEditor(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setChecked(bool checked);
	bool getChecked() const;

private slots:
	void checkedStateChangedEventHandler(Qt::CheckState);

	/////
	// Signals
signals:
	void checkedStateChangedEvent(CheckBoxEditor* me);

	/////
	// Object variables
private:
	QCheckBox* checkBox = nullptr;
	bool inhibitEvents = false;

};

#endif // CHECKBOXEDITOR_H
