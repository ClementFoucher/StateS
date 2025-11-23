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

#ifndef BLANKSCENE_H
#define BLANKSCENE_H

// Parent
#include "genericscene.h"

// Qt classes
class QGraphicsTextItem;


class BlankScene : public GenericScene
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit BlankScene();

protected:
	virtual void updateSimulationMode(SimulationMode_t) override;

	/////
	// Object variables
private:
	QGraphicsTextItem* text = nullptr;
	QFont defaultFont;

};

#endif // BLANKSCENE_H
