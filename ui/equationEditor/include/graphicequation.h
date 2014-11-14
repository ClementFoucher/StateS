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


#ifndef GRAPHICEQUATION_H
#define GRAPHICEQUATION_H

#include <QFrame>

#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>

class LogicVariable;

class GraphicEquation : public QFrame
{
    Q_OBJECT

public:
    explicit GraphicEquation(LogicVariable* equation, GraphicEquation* parent, bool isTemplate = false);

    void updateEquation();

    LogicVariable* getLogicEquation() const;

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
    void replaceEquation(LogicVariable* newVariable);
    void buildEquation();

    bool isTemplate = false;

    LogicVariable* equation = nullptr;

    GraphicEquation* parent = nullptr;

    QHBoxLayout* layout = nullptr;

    QLabel* operatorText = nullptr;
    QLabel* invertorText = nullptr;

    GraphicEquation* leftOperandDisplay  = nullptr;
    GraphicEquation* rightOperandDisplay = nullptr;

    QLabel* variableText = nullptr;

    LogicVariable* droppedEquation = nullptr;
};

#endif // GRAPHICEQUATION_H
