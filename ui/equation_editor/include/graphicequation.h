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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>

// StateS classes
class Signal;
class EquationEditor;
//class ContextMenu;


class GraphicEquation : public QFrame
{
    Q_OBJECT

private:
    enum CommonAction { Cancel = 0 };
    enum DropAction { ReplaceExisting = 1, ExistingAsOperand = 2};
    enum ContextAction { DeleteEquation = 3, IncrementOperandCount = 4, DecrementOperandCount = 5};

public:
    explicit GraphicEquation(shared_ptr<Signal> equation, bool isTemplate = false, QWidget* parent = nullptr);

    void updateEquation(shared_ptr<Signal> oldOperand, shared_ptr<Signal> newOperand);

    shared_ptr<Signal> getLogicEquation() const;

protected:
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

protected slots:
    void enterChildrenEventHandler();
    void leaveChildrenEventHandler();

private slots:
    void treatMenuEventHandler(QAction* action);

private:
    void setDefaultBorderColor();
    void setHilightedBorderColor();

    void replaceEquation(shared_ptr<Signal> newEquation);
    void buildEquation();

    // Use pointer because this is a QWidget with a parent
    GraphicEquation* parentEquation() const;

    bool isTemplate = false;

    weak_ptr<Signal> equation;
    // Only top-level GraphicEquation holds root Equation
    shared_ptr<Signal> rootEquation;

    shared_ptr<Signal> droppedEquation;
};

#endif // GRAPHICEQUATION_H
