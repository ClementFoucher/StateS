/*
 * Copyright © 2014-2026 Clément Foucher
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

// Qt
class QAbstractGraphicsShapeItem;

// StateS
#include "componentid.h"


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
	explicit GraphicComponent(ComponentId logicComponentId);
	virtual ~GraphicComponent() = default;

	/////
	// Object functions
public:
	ComponentId getLogicComponentId() const;

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
	ComponentId logicComponentId = nullId;

	QAbstractGraphicsShapeItem* selectionShape = nullptr;

};

#endif // GRAPHICCOMPONENT_H
