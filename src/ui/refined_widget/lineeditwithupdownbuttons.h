/*
 * Copyright © 2014-2015 Clément Foucher
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

#ifndef LINEEDITWITHUPDOWNBUTTONS_H
#define LINEEDITWITHUPDOWNBUTTONS_H

// Parent
#include <QWidget>

// Qt classes
class QIntValidator;

// StateS classes
class DynamicLineEdit;


class LineEditWithUpDownButtons : public QWidget
{
	Q_OBJECT

public:
	explicit LineEditWithUpDownButtons(int min, int max, const QString& text, QWidget* parent = nullptr);

	void updateContent(int min, int max, const QString& text);
	void edit();

signals:
	void valueChanged(int newValue);

protected:
	void wheelEvent(QWheelEvent* event) override;

private slots:
	void up();
	void down();

	void textUpdatedByUsedEventHandler(const QString& newText);

private:
	DynamicLineEdit* lineEdit  = nullptr;
	QIntValidator*   validator = nullptr;

};

#endif // LINEEDITWITHUPDOWNBUTTONS_H
