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

#ifndef EDITABLEEQUATION_H
#define EDITABLEEQUATION_H

// Parent
#include <QWidget>


class EditableEquation : public QWidget
{
    Q_OBJECT

public:
    explicit EditableEquation(QWidget* parent = nullptr);

    virtual bool validEdit() = 0;
    virtual bool cancelEdit() = 0;
    virtual void setEdited(bool edited) = 0;
};

#endif // EDITABLEEQUATION_H
