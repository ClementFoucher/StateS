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
#include "logicvalue.h"
class Signal;
class EditableEquation;


class GraphicEquation : public QFrame
{
    Q_OBJECT

private:
    enum CommonAction { Cancel = 0 };
    enum DropAction { ReplaceExisting = 1, ExistingAsOperand = 2};
    enum ContextAction { DeleteEquation = 3, IncrementOperandCount = 4, DecrementOperandCount = 5, ExtractSwitchSingle = 6, ExtractSwitchRange = 7, EditRange = 8, EditValue = 9};

public:
    explicit GraphicEquation(shared_ptr<Signal> equation, bool isTemplate = false, bool lockSignal = false, QWidget* parent = nullptr);

    void updateEquation(shared_ptr<Signal> oldOperand, shared_ptr<Signal> newOperand); // TODO: throw exception

    shared_ptr<Signal> getLogicEquation() const;

    void forceCompleteRendering();
    bool validEdit();
    bool cancelEdit();


protected:
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;

    void mousePressEvent      (QMouseEvent* event) override;
    void mouseMoveEvent       (QMouseEvent* event) override;
    void mouseReleaseEvent    (QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent     (QDropEvent*      event) override;

    void contextMenuEvent(QContextMenuEvent* event) override;

protected slots:
    void enterChildrenEventHandler();
    void leaveChildrenEventHandler();

private slots:
    void treatMenuEventHandler(QAction* action);
    void treatExtractIndex1Changed(int newIndex);
    void treatExtractIndex2Changed(int newIndex);
    void treatConstantValueChanged(LogicValue newValue);
    void updateBorder();

private:
    void setDefaultBorderColor();
    void setHilightedBorderColor();

    void replaceEquation(shared_ptr<Signal> newEquation);
    void buildEquation();
    void buildTemplateEquation();
    void buildSignalEquation();
    void buildCompleteEquation();

private:
    // Use pointer because this is a QWidget with a parent
    GraphicEquation* parentEquation() const;

    bool isTemplate = false;

    weak_ptr<Signal> equation;
    // Only top-level GraphicEquation holds root Equation
    shared_ptr<Signal> rootEquation;

    shared_ptr<Signal> droppedEquation;

    bool completeRendering = false;
    bool mouseIn = false;

    EditableEquation* editorWidget = nullptr;

    bool inMouseEvent = false;
    bool lockSignal;
};

#endif // GRAPHICEQUATION_H
