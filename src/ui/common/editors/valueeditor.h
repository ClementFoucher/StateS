/*
 * Copyright © 2025-2026 Clément Foucher
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

#ifndef VALUEEDITOR_H
#define VALUEEDITOR_H

// Parent
#include <QWidget>

// StateS classes
class LogicValue;
class ColoredLineEditor;


class ValueEditor : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ValueEditor(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setFocusOnShow(bool autoFocusOnNextShow);

	void setBitVectorValue(LogicValue value, uint size = 0); // Size 0 means no constraint on size
	LogicValue getBitVectorValue() const;

protected:
	void showEvent(QShowEvent* event) override;

	virtual void focusInEvent(QFocusEvent* event) override;

	virtual void keyPressEvent(QKeyEvent* event) override;

	/////
	// Signals
signals:
	void valueChangedEvent();
	void cancelEditEvent();

	/////
	// Object variables
private:
	ColoredLineEditor* lineEdit = nullptr;

	uint bitVectorSize = 0;
	bool autoFocusOnNextShow = false;

};

#endif // VALUEEDITOR_H
