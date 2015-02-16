/*
 * Copyright © 2014 Clément Foucher
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

#ifndef FSMGRAPHICALSTATE_H
#define FSMGRAPHICALSTATE_H

// Parents
#include <QObject>
#include <QGraphicsEllipseItem>

// Qt classes
#include <QBrush>
#include <QPen>
#include <QGraphicsTextItem>
#include <QAction>
#include <QPixmap>

// StateS classes
class FsmState;

class FsmGraphicalState : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

    // Static
public:
    static QPixmap getPixmap(uint size, bool isInitial = false, bool addArrow = false);

private:
    const static qreal radius;
    static QBrush inactiveBrush;
    static QBrush activeBrush;
    static QPen pen;

    // Private constructor
private:
    explicit FsmGraphicalState();

    // Public
public:
    explicit FsmGraphicalState(FsmState *logicState);
    ~FsmGraphicalState();

    FsmState* getLogicalState() const;

    static qreal getRadius();

    QGraphicsItemGroup* getActionsBox() const;

public slots:
    void rebuildRepresentation();

signals:
    void moving();
    void callEdit(FsmState* state);
    void callRename(FsmState* state);

    // Private
protected slots:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

public slots:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void treatMenu(QAction* action);

private:
    void askDelete();

    FsmState*           logicalState = nullptr;
    QGraphicsTextItem*  stateName    = nullptr;
    QGraphicsItemGroup* actionsBox   = nullptr;

};

#endif // FSMGRAPHICALSTATE_H
