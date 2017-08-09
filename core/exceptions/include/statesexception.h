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

#ifndef STATESEXCEPTION_H
#define STATESEXCEPTION_H

// C++ classes
using namespace std;
#include <exception>

// Qt classes
#include <QString>


class StatesException : exception
{

private:
	QString sourceClass;
	uint    errorEnumValue;
	string  errorText;

public:
	explicit StatesException(const QString& sourceClass, uint errorEnumValue, const QString& errorCause);

	QString getSourceClass() const;
	uint    getEnumValue() const;

	virtual const char* what() const noexcept override;
};

#endif // STATESEXCEPTION_H
