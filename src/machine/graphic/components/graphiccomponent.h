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

#ifndef GRAPHICCOMPONENT_H
#define GRAPHICCOMPONENT_H

// Parent
#include <QGraphicsObject>

// Qt classes
class QAbstractGraphicsShapeItem;

// StateS classes
#include "statestypes.h"


class GraphicComponent : public QGraphicsObject
{
	Q_OBJECT

	/////
	// Static variables
protected:
	static const int    defaultLineThickness;

	static const QColor defaultFillingColor;
	static const QColor defaultBorderColor;
	static const QColor hoverBorderColor;
	static const QColor drawingBorderColor;
	static const QColor underEditBorderColor;

	static const QBrush defaultBrush;
	static const QPen   defaultPen;

private:
	static const int    selectionLineThickness;

	static const QColor selectionShapeBorderColor;

	static const QPen   selectionShapePen;

	/////
	// Constructors/destructors
public:
	explicit GraphicComponent(componentId_t logicComponentId);
	virtual ~GraphicComponent() = default;

	/////
	// Object functions
public:
	componentId_t getLogicComponentId() const;

	virtual void refreshDisplay() = 0;

protected:
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

	void clearSelectionShape();
	void refreshSelectionShapeVisibility();

private:
	virtual QAbstractGraphicsShapeItem* buildSelectionShape() = 0;

	/////
	// Object variables
private:
	componentId_t logicComponentId = nullId;

	QAbstractGraphicsShapeItem* selectionShape = nullptr;

};

#endif // GRAPHICCOMPONENT_H
