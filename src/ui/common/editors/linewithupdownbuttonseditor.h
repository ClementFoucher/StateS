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

#ifndef LINEWITHUPDOWNBUTTONSEDITOR_H
#define LINEWITHUPDOWNBUTTONSEDITOR_H

// Parent
#include <QWidget>

// Qt classes
class QIntValidator;

// StateS classes
class DynamicLineEditor;


class LineWithUpDownButtonsEditor : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit LineWithUpDownButtonsEditor(int min, int max, const QString& text, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	int getValue() const;

	void setMinValue(int min);
	void setMaxValue(int max);

protected:
	virtual void wheelEvent(QWheelEvent* event) override;

	virtual void keyPressEvent  (QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	virtual void focusInEvent(QFocusEvent*) override;

private slots:
	void textUpdatedByUserEventHandler(const QString& newText);

private:
	void up();
	void down();

	/////
	// Signals
signals:
	void valueChanged(int newValue);

	/////
	// Object variables
private:
	DynamicLineEditor* lineEdit  = nullptr;
	QIntValidator*     validator = nullptr;

};

#endif // LINEWITHUPDOWNBUTTONSEDITOR_H
