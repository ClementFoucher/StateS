/*
 * Copyright © 2023 Clément Foucher
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

// Current object class
#include "graphicattributes.h"


GraphicAttributes::GraphicAttributes(QObject* parent) :
    QObject(parent)
{

}

void GraphicAttributes::addAttribute(uint componentId, attribute_t newAttribute)
{
	if (this->attributes.contains(componentId) == false)
	{
		this->attributes[componentId] = QList<attribute_t>();
	}

	this->attributes[componentId].append(newAttribute);
}

QList<attribute_t> GraphicAttributes::getAttributes(uint componentId)
{
	if (this->attributes.contains(componentId) == true)
	{
		return this->attributes[componentId];
	}
	else
	{
		return QList<attribute_t>();
	}
}
