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

// Current object class
#include "graphicattributes.h"


void GraphicAttributes::addAttribute(uint componentId, QString name, QString value)
{
	if (this->attributes.contains(componentId) == false)
	{
		this->attributes[componentId] = QMap<QString, QString>();
	}

	this->attributes[componentId][name] = value;
}

QString GraphicAttributes::getAttribute(uint componentId, QString name)
{
	if (this->attributes.contains(componentId) == true)
	{
		if (this->attributes[componentId].contains(name))
		{
			return this->attributes[componentId][name];
		}
	}

	return QString();
}
