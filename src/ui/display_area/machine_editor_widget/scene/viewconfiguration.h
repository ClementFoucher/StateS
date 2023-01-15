/*
 * Copyright © 2017-2023 Clément Foucher
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

#ifndef VIEWCONFIGURATION_H
#define VIEWCONFIGURATION_H

// Parent class
#include <QObject>

// Qt classes
#include <QPointF>


/**
 * @brief The ViewConfiguration class is used as a
 * temporary storage structure to transmit the view
 * characteristics when saving or loading to/from a file.
 */
class ViewConfiguration : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ViewConfiguration();

	/////
	// Object variables
public:
	QPointF sceneTranslation;
	QPointF viewCenter;
	qreal   zoomLevel;

};

#endif // VIEWCONFIGURATION_H
