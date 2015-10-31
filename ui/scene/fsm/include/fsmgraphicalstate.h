/*
 * Copyright © 2014-2015 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QAction;

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
    explicit FsmGraphicalState(shared_ptr<FsmState> logicState);
    ~FsmGraphicalState();

    shared_ptr<FsmState> getLogicalState() const;

    static qreal getRadius();

    QGraphicsItemGroup* getActionsBox() const;

    // Raised visibility (should be protected) to allow direct calls:
    // Needed cause an event can be manually transmitted to various items
    void keyPressEvent(QKeyEvent* event) override;

signals:
    void stateMovingEvent();
    void editStateCalledEvent(shared_ptr<FsmState> state);
    void renameStateCalledEvent(shared_ptr<FsmState> state);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private slots:
    void rebuildRepresentation();
    void treatMenu(QAction* action);

private:
    void askDelete();

    weak_ptr<FsmState> logicalState;

    QGraphicsTextItem*  stateName    = nullptr;
    QGraphicsItemGroup* actionsBox   = nullptr;

};

#endif // FSMGRAPHICALSTATE_H
