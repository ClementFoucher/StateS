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

#ifndef FSMGRAPHICSTATE_H
#define FSMGRAPHICSTATE_H

// Parents
#include "graphicactuator.h"
#include <QGraphicsEllipseItem>

// Qt classes
class QAction;

// StateS classes
#include "statestypes.h"


class FsmGraphicState : public GraphicActuator, public QGraphicsEllipseItem
{
	Q_OBJECT

	/////
	// Static functions
public:
	static qreal getRadius();
	static QPixmap getPixmap(uint size, bool isInitial = false, bool addArrow = false);

	/////
	// Static variables
private:
	const static qreal radius;
	static QBrush inactiveBrush;
	static QBrush activeBrush;
	static QPen pen;

	/////
	// Constructors/destructors
public:
	explicit FsmGraphicState(componentId_t logicComponentId);

	/////
	// Object functions
public:
	virtual void refreshDisplay() override;
	void enableMoveEvent();

	void moveState(Direction_t direction, bool smallMove);

	virtual QPainterPath shape()  const override;
	virtual QRectF boundingRect() const override;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
	void statePositionAboutToChangeEvent(componentId_t stateId);

	void editStateCalledEvent(componentId_t stateId);
	void renameStateCalledEvent(componentId_t stateId);
	void deleteStateCalledEvent(componentId_t stateId);
	void setInitialStateCalledEvent(componentId_t stateId);
	void beginDrawTransitionFromThisState(componentId_t stateId);

protected:
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private slots:
	void componentUpdatedEventHandler();
	void treatMenu(QAction* action);
	void machineModeChangedEventHandler(SimulationMode_t);

private:
	void rebuildRepresentation();
	void updateSelectionShapeDisplay();

	/////
	// Object variables
private:
	QGraphicsTextItem*    stateName      = nullptr;
	QGraphicsEllipseItem* selectionShape = nullptr;

	bool moveEventEnabled = false;

};

#endif // FSMGRAPHICSTATE_H
