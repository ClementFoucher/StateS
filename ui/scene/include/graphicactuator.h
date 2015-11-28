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

#ifndef GRAPHICACTUATOR_H
#define GRAPHICACTUATOR_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QGraphicsItemGroup;

// StateS classes
#include "machine.h"
class MachineActuatorComponent;


class GraphicActuator : public QObject
{
    Q_OBJECT

public:
    explicit GraphicActuator(QObject* parent = nullptr);
    ~GraphicActuator();

    QGraphicsItemGroup* getActionsBox() const;

protected:
    void setActuator(shared_ptr<MachineActuatorComponent> actuator);
    void buildActionsBox(const QPen& pen, bool center);

private:
    QGraphicsItemGroup* actionsBox = nullptr;
    weak_ptr<MachineActuatorComponent> actuator;

};

#endif // GRAPHICACTUATOR_H
