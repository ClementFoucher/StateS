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


#ifndef GRAPHICEQUATION_H
#define GRAPHICEQUATION_H

// Parent
#include <QFrame>

// Qt classes
#include <QMap>

// StateS classes
class Signal;
class EquationEditor;


class GraphicEquation : public QFrame
{
    Q_OBJECT

public:
    explicit GraphicEquation(Signal* equation, bool isTemplate = false, QWidget* parent = nullptr);

    void updateEquation(Signal *oldOperand, Signal *newOperand);

    Signal* getLogicEquation() const;

protected slots:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void treatMenu(QAction* action);

private:
    void replaceEquation(Signal* newEquation);
    void buildEquation();
    void childLeavedEvent();
    void childEnteredEvent();

    bool isTemplate = false;

    Signal* equation = nullptr;

    QMap<uint, GraphicEquation*> graphicOperands;

    Signal* droppedEquation = nullptr;
};

#endif // GRAPHICEQUATION_H
