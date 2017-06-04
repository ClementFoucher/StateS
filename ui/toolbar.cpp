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

// Current class header
#include "toolbar.h"

// StateS classes
#include "svgimagegenerator.h"


ToolBar::ToolBar(QWidget* parent) :
    QToolBar(parent)
{
    this->setIconSize(QSize(64, 64));

    // Build actions

    this->actionSaveAs = new QAction(this);
    this->actionSaveAs->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/save_as"))));
    this->actionSaveAs->setText(tr("Save"));
    this->actionSaveAs->setToolTip(tr("Save machine in a new file"));

    this->actionSave = new QAction(this);
    this->actionSave->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/save"))));
    this->actionSave->setText(tr("Save as"));
    this->actionSave->setToolTip(tr("Update saved file with current content") + " (" + tr("use ctrl+S shortcut to avoid confirm dialog") + ")");

    this->actionLoad = new QAction(this);
    this->actionLoad->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/load"))));
    this->actionLoad->setText(tr("Load"));
    this->actionLoad->setToolTip(tr("Load machine from file"));

    this->actionNewFsm = new QAction(this);
    //this->actionNewFsm->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/new_FSM"))));
    this->actionNewFsm->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/clear"))));
    this->actionNewFsm->setText(tr("New FSM"));
    this->actionNewFsm->setToolTip(tr("Create new FSM"));

   /* this->actionClear = new QAction(this);
    this->actionClear->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/clear"))));
    this->actionClear->setText(tr("Clear"));
    this->actionClear->setToolTip(tr("Clear machine"));*/

    this->actionExportImage = new QAction(this);
    this->actionExportImage->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/export_image"))));
    this->actionExportImage->setText(tr("Export to image file"));
    this->actionExportImage->setToolTip(tr("Export machine to an image file"));

    this->actionExportHdl = new QAction(this);
    this->actionExportHdl->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/export_VHDL"))));
    this->actionExportHdl->setText(tr("Export to VHDL"));
    this->actionExportHdl->setToolTip(tr("Export machine to VHDL"));

    this->actionUndo = new QAction(this);
    this->actionUndo->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/undo"))));
    this->actionUndo->setText(tr("Undo"));
    this->actionUndo->setToolTip(tr("Undo latest edit"));

    this->actionRedo = new QAction(this);
    this->actionRedo->setIcon(QIcon(SvgImageGenerator::getPixmapFromSvg(QString(":/icons/redo"))));
    this->actionRedo->setText(tr("Redo"));
    this->actionRedo->setToolTip(tr("Redo undone edit"));

    this->actionSaveAs->setEnabled(false);
    this->actionSave->setEnabled(false);
    this->actionExportImage->setEnabled(false);
    this->actionExportHdl->setEnabled(false);
    this->actionUndo->setEnabled(false);
    this->actionRedo->setEnabled(false);

    // Add actions
    this->addAction(this->actionSaveAs);
    this->addAction(this->actionSave);
    this->addAction(this->actionLoad);
    this->addSeparator();
    this->addAction(this->actionNewFsm);
    //this->addAction(this->actionClear);
    this->addSeparator();
    this->addAction(this->actionExportImage);
    this->addAction(this->actionExportHdl);
    this->addSeparator();
    this->addAction(this->actionUndo);
    this->addAction(this->actionRedo);


    // Connect actions
    connect(this->actionSaveAs,      &QAction::triggered, this, &ToolBar::saveAsRequestedEvent);
    connect(this->actionSave,        &QAction::triggered, this, &ToolBar::saveRequestedEvent);
    connect(this->actionLoad,        &QAction::triggered, this, &ToolBar::loadRequestedEvent);
    connect(this->actionNewFsm,      &QAction::triggered, this, &ToolBar::newMachineRequestedEvent);
    //connect(this->actionClear,       &QAction::triggered, this, &ToolBar::beginClearMachineProcedure);
    connect(this->actionExportImage, &QAction::triggered, this, &ToolBar::exportImageRequestedEvent);
    connect(this->actionExportHdl,   &QAction::triggered, this, &ToolBar::exportHdlRequestedEvent);
    connect(this->actionUndo,        &QAction::triggered, this, &ToolBar::undo);
    connect(this->actionRedo,        &QAction::triggered, this, &ToolBar::redo);
}

void ToolBar::setSaveAsActionEnabled(bool enable)
{
    this->actionSaveAs->setEnabled(enable);
}

void ToolBar::setSaveActionEnabled(bool enable)
{
    this->actionSave->setEnabled(enable);
}

void ToolBar::setExportActionsEnabled(bool enable)
{
    this->actionExportImage->setEnabled(enable);
    this->actionExportHdl  ->setEnabled(enable);
}

void ToolBar::setUndoActionEnabled(bool enable)
{
    this->actionUndo->setEnabled(enable);
}

void ToolBar::setRedoActionEnabled(bool enable)
{
    this->actionRedo->setEnabled(enable);
}
