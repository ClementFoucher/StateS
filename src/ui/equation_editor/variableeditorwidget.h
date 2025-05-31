/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VARIABLEEDITORWIDGET_H
#define VARIABLEEDITORWIDGET_H

// Parent
#include "equationparteditorwidget.h"

// States classes
#include "statestypes.h"


class VariableEditorWidget : public EquationPartEditorWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit VariableEditorWidget(componentId_t variableId, uint rankInParentOperands, bool isTemplate, QWidget* parent = nullptr); // Parent is mandatory if this is an operand of another equation

	/////
	// Object functions
protected:
	virtual uint    getAllowedMenuActions()   const override;
	virtual uint    getAllowedDropActions()   const override;
	virtual QString getText()                 const override;
	virtual QString getToolTipText()          const override;
	virtual bool    getReplaceWithoutAsking() const override;
	virtual bool    getIsErroneous()          const override;

	virtual QDrag* buildDrag() override;

	virtual void processSpecificMenuAction(ContextAction_t action) override;
	virtual void processSpecificDropAction(DropAction_t action)    override;

	/////
	// Object variables
private:
	componentId_t variableId = nullId;

};

#endif // VARIABLEEDITORWIDGET_H
