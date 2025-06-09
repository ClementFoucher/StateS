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

// Current class header
#include "graphiccomponent.h"

// Qt classes
#include <QPen>


//
// Static members
//

// Protected
const int GraphicComponent::defaultLineThickness = 3;

const QColor GraphicComponent::defaultFillingColor  = QColor(230,  230, 230);
const QColor GraphicComponent::defaultBorderColor   = QColor(0, 0, 0);
const QColor GraphicComponent::hoverBorderColor     = Qt::blue;
const QColor GraphicComponent::drawingBorderColor   = Qt::blue;
const QColor GraphicComponent::underEditBorderColor = Qt::gray;

const QBrush GraphicComponent::defaultBrush = QBrush(GraphicComponent::defaultFillingColor, Qt::SolidPattern);
const QPen   GraphicComponent::defaultPen   = QPen(QBrush(GraphicComponent::defaultBorderColor, Qt::SolidPattern), GraphicComponent::defaultLineThickness);

// Private
const int GraphicComponent::selectionLineThickness = 1;

const QColor GraphicComponent::selectionShapeBorderColor = QColor(0, 0, 204, 200);

const QPen GraphicComponent::selectionShapePen = QPen(QBrush(GraphicComponent::selectionShapeBorderColor, Qt::SolidPattern), GraphicComponent::selectionLineThickness, Qt::DashLine);


//
// Class object definition
//

GraphicComponent::GraphicComponent(componentId_t logicComponentId)
{
	this->logicComponentId = logicComponentId;
}

componentId_t GraphicComponent::getLogicComponentId() const
{
	return this->logicComponentId;
}

void GraphicComponent::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{

}

QVariant GraphicComponent::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged)
	{
		this->refreshSelectionShapeVisibility();
	}

	return QGraphicsItem::itemChange(change, value);
}

void GraphicComponent::clearSelectionShape()
{
	delete this->selectionShape;
	this->selectionShape = nullptr;
}

void GraphicComponent::refreshSelectionShapeVisibility()
{
	if (this->isSelected() == true)
	{
		if (this->selectionShape == nullptr)
		{
			this->selectionShape = this->buildSelectionShape();
			this->selectionShape->setPen(selectionShapePen);
		}
		else
		{
			this->selectionShape->setVisible(true);
		}
	}
	else if ( (this->isSelected() == false) && (this->selectionShape != nullptr) )
	{
		this->selectionShape->setVisible(false);
	}
}
