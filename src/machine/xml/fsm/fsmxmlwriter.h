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

#ifndef FSMXMLWRITER_H
#define FSMXMLWRITER_H

// Parent class
#include "machinexmlwriter.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class ViewConfiguration;
class Fsm;
class GraphicAttributes;


class FsmXmlWriter : public MachineXmlWriter
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmXmlWriter(MachineXmlWriterMode_t mode, shared_ptr<ViewConfiguration> viewConfiguration = nullptr);

	/////
	// Object functions
protected:
	virtual void writeSubmachineToStream() override;
	virtual void writeMachineType() override;

private:
	void writeFsmStates(shared_ptr<Fsm> fsm, shared_ptr<GraphicAttributes> fsmGraphicAttributes);
	void writeFsmTransitions(shared_ptr<Fsm> fsm, shared_ptr<GraphicAttributes> fsmGraphicAttributes);

};

#endif // FSMXMLWRITER_H
