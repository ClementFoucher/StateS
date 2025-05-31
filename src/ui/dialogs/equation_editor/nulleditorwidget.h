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

#ifndef NULLEDITORWIDGET_H
#define NULLEDITORWIDGET_H

// Parent
#include "equationparteditorwidget.h"


class NullEditorWidget : public EquationPartEditorWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit NullEditorWidget(uint rankInParentOperands, QWidget* parent = nullptr); // Parent is mandatory if this is an operand of another equation

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

	virtual void processSpecificMenuAction(ContextAction_t) override;
	virtual void processSpecificDropAction(DropAction_t)    override;

};

#endif // NULLEDITORWIDGET_H
