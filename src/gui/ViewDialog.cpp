/*
 * Stellarium
 * Copyright (C) 2008 Fabien Chereau
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
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "ViewDialog.hpp"
#include "ui_viewDialog.h"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "Mapping.hpp"
#include "Projector.hpp"
#include "LandscapeMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StarMgr.hpp"
#include "SkyDrawer.hpp"
#include "SolarSystem.hpp"
#include "NebulaMgr.hpp"
#include "MeteorMgr.hpp"
#include "GridLinesMgr.hpp"
#include "ConstellationMgr.hpp"

#include <QDebug>
#include <QFrame>
#include <QFile>

#include "StelMainGraphicsView.hpp"
#include <QDialog>

ViewDialog::ViewDialog()
{
	ui = new Ui_viewDialogForm;
}

ViewDialog::~ViewDialog()
{
	delete ui;
}

void ViewDialog::languageChanged()
{
	if (dialog)
	{
		ui->retranslateUi(dialog);
		shoutingStarsZHRChanged();
		populateLists();
	}
}

void ViewDialog::createDialogContent()
{
	ui->setupUi(dialog);
	connect(ui->closeView, SIGNAL(clicked()), this, SLOT(close()));
	
	populateLists();
	connect(ui->culturesListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(skyCultureChanged(const QString&)));
	connect(ui->projectionListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(projectionChanged(const QString&)));
	connect(ui->landscapesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(landscapeChanged(QListWidgetItem*)));
	
	// Connect and initialize checkboxes and other widgets
	
	// Stars section
	QAction* a;
	StarMgr* smgr = (StarMgr*)GETSTELMODULE("StarMgr");
// 	ui->showStarsCheckBox->setChecked(smgr->getFlagStars());
// 	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Stars");
// 	connect(a, SIGNAL(toggled(bool)), ui->showStarsCheckBox, SLOT(setChecked(bool)));
// 	connect(ui->showStarsCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->starTwinkleCheckBox->setChecked(StelApp::getInstance().getCore()->getSkyDrawer()->getFlagTwinkle());
	connect(ui->starTwinkleCheckBox, SIGNAL(toggled(bool)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setFlagTwinkle(bool)));
	
	ui->starScaleRadiusDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getAbsoluteStarScale());
	connect(ui->starScaleRadiusDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setAbsoluteStarScale(double)));
	
	ui->starRelativeScaleDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getRelativeStarScale());
	connect(ui->starRelativeScaleDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setRelativeStarScale(double)));	
	
	ui->starTwinkleAmountDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getTwinkleAmount());
	connect(ui->starTwinkleAmountDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setTwinkleAmount(double)));
	
	ui->adaptationCheckbox->setChecked(StelApp::getInstance().getCore()->getSkyDrawer()->getFlagLuminanceAdaptation());
	connect(ui->adaptationCheckbox, SIGNAL(toggled(bool)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setFlagLuminanceAdaptation(bool)));
	
	// Planets section
	SolarSystem* ssmgr = (SolarSystem*)GETSTELMODULE("SolarSystem");
	ui->showPlanetCheckBox->setChecked(ssmgr->getFlagPlanets());
	connect(ui->showPlanetCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagPlanets(bool)));
	
	ui->planetMarkerCheckBox->setChecked(ssmgr->getFlagHints());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Planets_Hints");
	connect(a, SIGNAL(toggled(bool)), ui->planetMarkerCheckBox, SLOT(setChecked(bool)));
	connect(ui->planetMarkerCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->planetScaleMoonCheckBox->setChecked(ssmgr->getFlagMoonScale());
	connect(ui->planetScaleMoonCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagMoonScale(bool)));
	
	ui->planetOrbitCheckBox->setChecked(ssmgr->getFlagOrbits());
	connect(ui->planetOrbitCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagOrbits(bool)));
	
	ui->planetLightSpeedCheckBox->setChecked(ssmgr->getFlagLightTravelTime());
	connect(ui->planetLightSpeedCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagLightTravelTime(bool)));
	
	// Shouting stars section
	MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE("MeteorMgr");
	ui->zhrNone->setChecked(!mmgr->getFlagShow());
	connect(ui->zhrNone, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
	connect(ui->zhr10, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
	connect(ui->zhr80, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
	connect(ui->zhr10000, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
	connect(ui->zhr144000, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
	
	// Labels section
	NebulaMgr* nmgr = (NebulaMgr*)GETSTELMODULE("NebulaMgr");
	ui->starLabelCheckBox->setChecked(smgr->getFlagLabels());
	connect(ui->starLabelCheckBox, SIGNAL(toggled(bool)), smgr, SLOT(setFlagLabels(bool)));
	ui->planetLabelCheckBox->setChecked(ssmgr->getFlagLabels());
	connect(ui->planetLabelCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagLabels(bool)));
	ui->nebulaLabelCheckBox->setChecked(nmgr->getFlagHints());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Nebulas");
	connect(a, SIGNAL(toggled(bool)), ui->nebulaLabelCheckBox, SLOT(setChecked(bool)));
	connect(ui->nebulaLabelCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->starsLabelsHorizontalSlider->setValue((int)(smgr->getLabelsAmount()*10.f));
	connect(ui->starsLabelsHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(starsLabelsValueChanged(int)));
	ui->planetsLabelsHorizontalSlider->setValue((int)(ssmgr->getLabelsAmount()*10.f));
	connect(ui->planetsLabelsHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(planetsLabelsValueChanged(int)));
	ui->nebulasLabelsHorizontalSlider->setValue((int)(nmgr->getHintsAmount()*10.f));
	connect(ui->nebulasLabelsHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(nebulasLabelsValueChanged(int)));
	
	// Landscape section
	LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE("LandscapeMgr");
	ui->showGroundCheckBox->setChecked(lmgr->getFlagLandscape());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Ground");
	connect(a, SIGNAL(toggled(bool)), ui->showGroundCheckBox, SLOT(setChecked(bool)));
	connect(ui->showGroundCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showFogCheckBox->setChecked(lmgr->getFlagFog());
	connect(ui->showFogCheckBox, SIGNAL(toggled(bool)), lmgr, SLOT(setFlagFog(bool)));
	
	ui->showAtmosphereCheckBox->setChecked(lmgr->getFlagAtmosphere());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Atmosphere");
	connect(a, SIGNAL(toggled(bool)), ui->showAtmosphereCheckBox, SLOT(setChecked(bool)));
	connect(ui->showAtmosphereCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->landscapePositionCheckBox->setChecked(lmgr->getFlagLandscapeSetsLocation());
	connect(ui->landscapePositionCheckBox, SIGNAL(toggled(bool)), lmgr, SLOT(setFlagLandscapeSetsLocation(bool)));
	
	ui->lightPollutionSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getBortleScale());
	connect(ui->lightPollutionSpinBox, SIGNAL(valueChanged(int)), lmgr, SLOT(setAtmosphereBortleLightPollution(int)));
	connect(ui->lightPollutionSpinBox, SIGNAL(valueChanged(int)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setBortleScale(int)));
	
	// Grid and lines
	GridLinesMgr* glmgr = (GridLinesMgr*)GETSTELMODULE("GridLinesMgr");
	ui->showEquatorLineCheckBox->setChecked(glmgr->getFlagEquatorLine());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Equator_Line");
	connect(a, SIGNAL(toggled(bool)), ui->showEquatorLineCheckBox, SLOT(setChecked(bool)));
	connect(ui->showEquatorLineCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showEclipticLineCheckBox->setChecked(glmgr->getFlagEclipticLine());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Ecliptic_Line");
	connect(a, SIGNAL(toggled(bool)), ui->showEclipticLineCheckBox, SLOT(setChecked(bool)));
	connect(ui->showEclipticLineCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showMeridianLineCheckBox->setChecked(glmgr->getFlagMeridianLine());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Meridian_Line");
	connect(a, SIGNAL(toggled(bool)), ui->showMeridianLineCheckBox, SLOT(setChecked(bool)));
	connect(ui->showMeridianLineCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showEquatorialGridCheckBox->setChecked(glmgr->getFlagEquatorGrid());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Equatorial_Grid");
	connect(a, SIGNAL(toggled(bool)), ui->showEquatorialGridCheckBox, SLOT(setChecked(bool)));
	connect(ui->showEquatorialGridCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showAzimutalGridCheckBox->setChecked(glmgr->getFlagAzimutalGrid());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Azimutal_Grid");
	connect(a, SIGNAL(toggled(bool)), ui->showAzimutalGridCheckBox, SLOT(setChecked(bool)));
	connect(ui->showAzimutalGridCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showEquatorialJ2000GridCheckBox->setChecked(glmgr->getFlagEquatorJ2000Grid());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Equatorial_J2000_Grid");
	connect(a, SIGNAL(toggled(bool)), ui->showEquatorialJ2000GridCheckBox, SLOT(setChecked(bool)));
	connect(ui->showEquatorialJ2000GridCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showCardinalPointsCheckBox->setChecked(lmgr->getFlagCardinalsPoints());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Cardinal_Points");
	connect(a, SIGNAL(toggled(bool)), ui->showCardinalPointsCheckBox, SLOT(setChecked(bool)));
	connect(ui->showCardinalPointsCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	// Constellations
	ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE("ConstellationMgr");
	
	ui->showConstellationLinesCheckBox->setChecked(cmgr->getFlagLines());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Constellation_Lines");
	connect(a, SIGNAL(toggled(bool)), ui->showConstellationLinesCheckBox, SLOT(setChecked(bool)));
	connect(ui->showConstellationLinesCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showConstellationLabelsCheckBox->setChecked(cmgr->getFlagLabels());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Constellation_Labels");
	connect(a, SIGNAL(toggled(bool)), ui->showConstellationLabelsCheckBox, SLOT(setChecked(bool)));
	connect(ui->showConstellationLabelsCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showConstellationBoundariesCheckBox->setChecked(cmgr->getFlagBoundaries());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Constellation_Boundaries");
	connect(a, SIGNAL(toggled(bool)), ui->showConstellationBoundariesCheckBox, SLOT(setChecked(bool)));
	connect(ui->showConstellationBoundariesCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->showConstellationArtCheckBox->setChecked(cmgr->getFlagArt());
	a = StelMainGraphicsView::getInstance().findChild<QAction*>("actionShow_Constellation_Art");
	connect(a, SIGNAL(toggled(bool)), ui->showConstellationArtCheckBox, SLOT(setChecked(bool)));
	connect(ui->showConstellationArtCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
	
	ui->constellationArtBrightnessSpinBox->setValue(cmgr->getArtIntensity());
	connect(ui->constellationArtBrightnessSpinBox, SIGNAL(valueChanged(double)), cmgr, SLOT(setArtIntensity(double)));
}

void ViewDialog::populateLists()
{
	// Fill the culture list widget from the available list
	QListWidget* l = ui->culturesListWidget;
	l->blockSignals(true);
	l->clear();
	l->addItems(StelApp::getInstance().getSkyCultureMgr().getSkyCultureListI18());
	l->setCurrentItem(l->findItems(StelApp::getInstance().getSkyCultureMgr().getSkyCultureNameI18(), Qt::MatchExactly).at(0));
	l->blockSignals(false);
	updateSkyCultureText();

	// Fill the projection list
	l = ui->projectionListWidget;
	l->blockSignals(true);
	l->clear();
	const QMap<QString, const Mapping*>& mappings = StelApp::getInstance().getCore()->getProjection()->getAllMappings();
	QMapIterator<QString, const Mapping*> i(mappings);
	while (i.hasNext())
	{
		i.next();
		l->addItem(i.value()->getNameI18());
	}
	l->setCurrentItem(l->findItems(StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getNameI18(), Qt::MatchExactly).at(0));
	l->blockSignals(false);
	ui->projectionTextBrowser->setHtml(StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getHtmlSummary());

	// Fill the landscape list
	l = ui->landscapesListWidget;
	l->blockSignals(true);
	l->clear();
	LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE("LandscapeMgr");
	l->addItems(lmgr->getAllLandscapeNames());
	l->setCurrentItem(l->findItems(lmgr->getCurrentLandscapeName(), Qt::MatchExactly).at(0));
	l->blockSignals(false);
	ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());
}

void ViewDialog::skyCultureChanged(const QString& cultureName)
{
	StelApp::getInstance().getSkyCultureMgr().setSkyCulture(cultureName);
	updateSkyCultureText();
}

void ViewDialog::updateSkyCultureText()
{	
	StelFileMgr& fileMan(StelApp::getInstance().getFileMgr());
	QString descPath;
	try
	{
		descPath = fileMan.findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getSkyCultureDir() + "/description."+StelApp::getInstance().getLocaleMgr().getAppLanguage()+".utf8");
	}
	catch (exception& e)
	{
		try
		{
			descPath = fileMan.findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getSkyCultureDir() + "/description.en.utf8");
		}
		catch (exception& e)
		{
			qWarning() << "WARNING: can't find description for skyculture" << StelApp::getInstance().getSkyCultureMgr().getSkyCultureDir();
		}
	}
	if (descPath.isEmpty())
	{
		ui->skyCultureTextBrowser->setHtml(q_("No description"));
	}
	else
	{
		QFile f(descPath);
		f.open(QIODevice::ReadOnly);
		ui->skyCultureTextBrowser->setHtml(QString::fromUtf8(f.readAll()));
	}
}

void ViewDialog::projectionChanged(const QString& projectionName)
{
	const QMap<QString, const Mapping*>& mappings = StelApp::getInstance().getCore()->getProjection()->getAllMappings();
	QMapIterator<QString, const Mapping*> i(mappings);
	while (i.hasNext())
	{
		i.next();
		if (i.value()->getNameI18() == projectionName)
			break;
	}

	StelApp::getInstance().getCore()->getProjection()->setCurrentMapping(i.value()->getId());
	ui->projectionTextBrowser->setHtml(StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getHtmlSummary());
}

void ViewDialog::landscapeChanged(QListWidgetItem* item)
{
	LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE("LandscapeMgr");
	lmgr->setLandscapeByName(item->text());
	ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());
}

void ViewDialog::shoutingStarsZHRChanged()
{
	MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE("MeteorMgr");
	int zhr;
	if (ui->zhrNone->isChecked())
	{
		mmgr->setFlagShow(false);
		zhr = 0;
	}
	else
	{
		mmgr->setFlagShow(true);
	}
	if (ui->zhr10->isChecked())
		zhr = 10;
	if (ui->zhr80->isChecked())
		zhr = 80;
	if (ui->zhr10000->isChecked())
		zhr = 10000;
	if (ui->zhr144000->isChecked())
		zhr = 144000;
	if (zhr!=mmgr->getZHR())
		mmgr->setZHR(zhr);
	switch (zhr)
	{
		case 0:
			ui->zhrLabel->setText(q_("No shooting stars"));
			break;
		case 10:
			ui->zhrLabel->setText(q_("Normal rate"));
			break;
		case 80:
			ui->zhrLabel->setText(q_("Standard Perseids rate"));
			break;
		case 10000:
			ui->zhrLabel->setText(q_("Exceptional Leonid rate"));
			break;
		case 144000:
			ui->zhrLabel->setText(q_("Highest rate ever (1966 Leonids)"));
			break;
	}
}

void ViewDialog::starsLabelsValueChanged(int v)
{
	StarMgr* smgr = (StarMgr*)GETSTELMODULE("StarMgr");
	float a= ((float)v)/10.f;
	smgr->setLabelsAmount(a);
}

void ViewDialog::planetsLabelsValueChanged(int v)
{
	SolarSystem* ssmgr = (SolarSystem*)GETSTELMODULE("SolarSystem");
	float a= ((float)v)/10.f;
	ssmgr->setLabelsAmount(a);
}

void ViewDialog::nebulasLabelsValueChanged(int v)
{
	NebulaMgr* nmgr = (NebulaMgr*)GETSTELMODULE("NebulaMgr");
	float a= ((float)v)/10.f;
	nmgr->setHintsAmount(a);
	nmgr->setLabelsAmount(a);
}
