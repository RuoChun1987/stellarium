/*
 * Stellarium
 * Copyright (C) 2016 Alexander Wolf
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
*/

#include "StelApp.hpp"
#include "StelModuleMgr.hpp"
#include "StelUtils.hpp"
#include "GreatRedSpotDialog.hpp"
#include "ui_greatRedSpotDialog.h"

GreatRedSpotDialog::GreatRedSpotDialog()
{
	dialogName = "GreatRedSpot";
	ui = new Ui_GreatRedSpotDialogForm;
}

GreatRedSpotDialog::~GreatRedSpotDialog()
{
	delete ui;
}

void GreatRedSpotDialog::retranslate()
{
	if (dialog)
		ui->retranslateUi(dialog);
}


void GreatRedSpotDialog::createDialogContent()
{
	ui->setupUi(dialog);
	
	//Signals and slots
	connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslate()));
	connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->TitleBar, SIGNAL(movedTo(QPoint)), this, SLOT(handleMovedTo(QPoint)));

	SolarSystem* ss = GETSTELMODULE(SolarSystem);
	ui->longitudeSpinBox->setValue(ss->getCustomGrsLongitude());
	connect(ui->longitudeSpinBox, SIGNAL(valueChanged(int)), ss, SLOT(setCustomGrsLongitude(int)));

	ui->driftDoubleSpinBox->setValue(ss->getCustomGrsDrift());
	connect(ui->driftDoubleSpinBox, SIGNAL(valueChanged(double)), ss, SLOT(setCustomGrsDrift(double)));

	//TODO: sync format with the main date and time format
	ui->jdDateTimeEdit->setDisplayFormat("yyyy.MM.dd hh:mm");
	ui->jdDateTimeEdit->setDateTime(StelUtils::jdToQDateTime(ss->getCustomGrsJD()));
	connect(ui->jdDateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(setGrsJD(QDateTime)));
}

void GreatRedSpotDialog::setGrsJD(QDateTime dt)
{
	GETSTELMODULE(SolarSystem)->setCustomGrsJD(StelUtils::qDateTimeToJd(dt));
}
