/*
 * Copyright © 2017 Clément Foucher
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

#ifndef FSMUNDOCOMMAND_H
#define FSMUNDOCOMMAND_H

// Parent class
#include "machineundocommand.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QPointF>

// StateS classes
class FsmState;


class FsmUndoCommand : public MachineUndoCommand
{
	Q_OBJECT

public:
	explicit FsmUndoCommand(shared_ptr<FsmState> state);

	virtual void undo() override;
	virtual void redo() override;

	virtual bool mergeWith(const QUndoCommand* command) override;

	QString getComponentRef() const;
	QPointF getNextPosition() const;

private:
	QString componentRef;

	QPointF previousPosition;
	QPointF nextPosition;
};

#endif // FSMUNDOCOMMAND_H
