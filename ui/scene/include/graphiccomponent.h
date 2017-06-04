/*
 * Copyright © 2014-2017 Clément Foucher
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

#ifndef GRAPHICCOMPONENT_H
#define GRAPHICCOMPONENT_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class MachineComponent;


class GraphicComponent : public QObject
{
    Q_OBJECT

public: // Static
    enum GraphicComponentErrorEnum{
        reaffect_component   = 0,
        obsolete_base_object = 1
    };

public:
    explicit GraphicComponent(QObject* parent = nullptr);

protected:
    void setLogicComponent(shared_ptr<MachineComponent> logicComponent); // Throws StatesException
    shared_ptr<MachineComponent> getLogicComponent() const;

private:
    weak_ptr<MachineComponent> logicComponent;

protected:
    static QPen selectionPen;

};

#endif // GRAPHICCOMPONENT_H
