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

#ifndef TRANSITIONEDITORTAB_H
#define TRANSITIONEDITORTAB_H

// Parent
#include "componenteditortab.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class FsmTransition;
class ActionEditor;
class ConditionEditor;


class TransitionEditorTab : public ComponentEditorTab
{
    Q_OBJECT

public:
    explicit TransitionEditorTab(shared_ptr<FsmTransition> transition, QWidget* parent = nullptr);

    void changeEditedTransition(shared_ptr<FsmTransition> transition);

private:
    // QWidgets with parent
    ActionEditor*    actionEditor    = nullptr;
    ConditionEditor* conditionEditor = nullptr;
};

#endif // TRANSITIONEDITORTAB_H
