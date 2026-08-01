/*
 * Copyright © 2026 Clément Foucher
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

#ifndef VARIABLETABLESCENE_H
#define VARIABLETABLESCENE_H

// Parent
#include "statesscene.h"

// Stdlib
#include <memory>
using namespace std;

// StateS
#include "statestypes.h"
class Variable;


class VariableTableScene : public StatesScene
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit VariableTableScene() = default;

	/////
	// Object functions
public:
	void buildScene(bool displayInputs, bool displayOutputs, bool displayVariables, bool displayConstants);

private:
	QGraphicsTextItem* buildSectionTitle(const QString& titleText, qreal verticalPos);
	qreal buildColumn(const QString& header, const QList<componentId_t> variablesIds, qreal horizontalPos, qreal verticalPos, function<QString(shared_ptr<Variable> variable)> textFunc);

	void drawVerticalLine(qreal x, qreal y1, qreal y2);
	void centerAndFrameTitle(QGraphicsTextItem* title);
	void frameTable(qreal sectionTop, qreal sectionSubHeader, qreal sectionBottom);

	/////
	// Object variables
private:
	const qreal vSpace = 10;

	qreal rightestHorizontalPos;
	qreal lowestVerticalPos;

};

#endif // VARIABLETABLESCENE_H
