/*
 * Copyright © 2023-2025 Clément Foucher
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

#ifndef GRAPHICATTRIBUTES_H
#define GRAPHICATTRIBUTES_H

// Parent class
#include <QObject>

// Qt classes
#include <QMap>

// StateS classes
#include "statestypes.h"


class GraphicAttributes : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicAttributes(QObject* parent = nullptr) : QObject(parent) {}

	/////
	// Object functions
public:
	void addAttribute(uint componentId, QString name, QString value);
	QString getAttribute(uint componentId, QString name);

	/////
	// Object variables
private:
	QMap<componentId_t, QMap<QString, QString>> attributes;

};

#endif // GRAPHICATTRIBUTES_H
