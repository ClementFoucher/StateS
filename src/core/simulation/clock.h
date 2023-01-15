/*
 * Copyright © 2014-2023 Clément Foucher
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

#ifndef CLOCK_H
#define CLOCK_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QTimer;


class Clock : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Clock();

	/////
	// Object functions
public:
	void start(uint intervalms);
	void stop();
	void reset();

public slots:
	void nextStep();

signals:
	void prepareForClockEvent();
	void clockEvent();

	void resetLogicEvent();
	void resetGraphicEvent();

	/////
	// Object variables
private:
	shared_ptr<QTimer> timer;

};

#endif // CLOCK_H
