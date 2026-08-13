/*
 * Copyright © 2025-2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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

#ifndef FSMTRANSITIONCONDITIONSLIDERPOSITIONCHANGEUNDOCOMMAND_H
#define FSMTRANSITIONCONDITIONSLIDERPOSITIONCHANGEUNDOCOMMAND_H

// Parent
#include "statesundocommand.h"

// StateS
#include "componentid.h"


class FsmTransitionConditionSliderPositionChangeUndoCommand : public StatesUndoCommand
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmTransitionConditionSliderPositionChangeUndoCommand(ComponentId componentId);

	/////
	// Object functions
public:
	virtual void undo() override;
	virtual void redo() override;

	virtual bool mergeWith(const QUndoCommand* command) override;

	/////
	// Object variables
private:
	ComponentId componentId = nullId;

	qreal previousSliderPosition;
	qreal nextTransitionSliderPosition;

};

#endif // FSMTRANSITIONCONDITIONSLIDERPOSITIONCHANGEUNDOCOMMAND_H
