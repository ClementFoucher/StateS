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

#ifndef GRAPHICFSMSTATE_H
#define GRAPHICFSMSTATE_H

// Parents
#include "graphiccomponent.h"
#include <QGraphicsEllipseItem>

// Qt classes
class QAction;

// StateS classes
#include "statestypes.h"
class ActionBox;


class GraphicFsmState : public GraphicComponent, public QGraphicsEllipseItem
{
	Q_OBJECT

	/////
	// Static functions
public:
	static qreal getRadius();
	static QPixmap getPixmap(uint size, bool isInitial = false, bool addArrow = false);

	/////
	// Static variables
protected:
	static const qreal  radius;
	static const QBrush defaultBrush;
	static const QPen   defaultPen;
	static const QPen   hoverPen;

	/////
	// Constructors/destructors
public:
	explicit GraphicFsmState(componentId_t logicComponentId);
	~GraphicFsmState();

	/////
	// Object functions
public:
	virtual void refreshDisplay() override;

	virtual QPainterPath shape()  const override;
	virtual QRectF boundingRect() const override;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	ActionBox* getActionBox() const;

signals:
	void statePositionAboutToChangeEvent(componentId_t stateId);
	void statePositionChangedEvent(componentId_t stateId);

	void editStateCalledEvent(componentId_t stateId);
	void renameStateCalledEvent(componentId_t stateId);
	void deleteStateCalledEvent(componentId_t stateId);
	void setInitialStateCalledEvent(componentId_t stateId);
	void beginDrawTransitionFromThisState(componentId_t stateId);

protected:
	virtual void keyPressEvent(QKeyEvent* event)                                  override;
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event)          override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event)                 override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)                 override;

private slots:
	void treatMenu(QAction* action);

private:
	void clearRepresentation();
	void buildRepresentation();
	void updateSelectionShapeDisplay();

	void updateActionBoxPosition();

	/////
	// Object variables
private:
	QGraphicsTextItem*    stateName      = nullptr;
	QGraphicsEllipseItem* selectionShape = nullptr;
	ActionBox*            actionBox      = nullptr;

};

#endif // GRAPHICFSMSTATE_H
