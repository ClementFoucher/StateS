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

#ifndef EQUATIONEDITORWIDGET_H
#define EQUATIONEDITORWIDGET_H

// Parent
#include "equationparteditorwidget.h"

// StateS classes
class RangeEditor;


class EquationEditorWidget : public EquationPartEditorWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit EquationEditorWidget(shared_ptr<Equation> equation, uint rankInParentOperands, bool isTemplate, QWidget* parent = nullptr); // Parent is mandatory if this is an operand of another equation

	/////
	// Object functions
public:
	void replaceOperand(uint operandRank, shared_ptr<Equation> newOperand);
	void replaceOperand(uint operandRank, LogicValue newConstant, bool isProcessingDrop = false);
	void replaceOperand(uint operandRank, componentId_t newVariable);
	void clearOperand(uint operandRank);

	shared_ptr<Equation> getLogicEquation() const;

	void closeOpenEditors();

	virtual void beginEdit()  override;
	virtual bool validEdit()  override;
	virtual bool cancelEdit() override;

protected:
	virtual uint    getAllowedMenuActions()   const override;
	virtual uint    getAllowedDropActions()   const override;
	virtual QString getText()                 const override;
	virtual QString getToolTipText()          const override;
	virtual bool    getReplaceWithoutAsking() const override;
	virtual bool    getIsErroneous()          const override;

	virtual QDrag* buildDrag() override;

	virtual void processSpecificMenuAction(ContextAction_t action) override;
	virtual void processSpecificDropAction(DropAction_t    action) override;

private slots:
	void rangeEditorBeginEditEventHandler();
	void equationChangedEventHandler();

private:
	void clear();
	void buildTemplateEquation();
	void buildCompleteEquation();
	void updateOperandWidget(uint operandRank);
	void setInverted(bool invert);
	void fixExtractorRange();
	EquationPartEditorWidget* buildOperandEditorWidget(uint operandRank);

	QString getTemplateText() const;

	/////
	// Object variables
private:
	shared_ptr<Equation> equation;

	QList<EquationPartEditorWidget*> operands;
	RangeEditor* rangeEditor = nullptr;

};

#endif // EQUATIONEDITORWIDGET_H
