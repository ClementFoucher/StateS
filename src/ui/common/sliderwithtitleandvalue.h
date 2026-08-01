/*
 * Copyright © 2026 Clément Foucher
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

#ifndef SLIDERWITHTITLEANDVALUE_H
#define SLIDERWITHTITLEANDVALUE_H

// Parent
#include <QWidget>

// Qt
class QLabel;


class SliderWithTitleAndValue : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SliderWithTitleAndValue(int minValue, int maxValue, int initialValue, const QString& title, const QString& valuePrefix, const QString& valueSuffix, QWidget* parent = nullptr);

	/////
	// Object functions
private slots:
	void sliderValueChangedEventHandler(int newValue);

	/////
	// Signals
signals:
	void valueChangedEvent(int newValue);

	/////
	// Object variables
private:
	QString valuePrefix;
	QString valueSuffix;

	QLabel* valueLabel = nullptr;

};

#endif // SLIDERWITHTITLEANDVALUE_H
