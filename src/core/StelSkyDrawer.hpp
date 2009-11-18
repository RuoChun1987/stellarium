/*
 * Stellarium
 * Copyright (C) 2007 Fabien Chereau
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

#ifndef _STELSKYDRAWER_HPP_
#define _STELSKYDRAWER_HPP_

#include "StelTextureTypes.hpp"
#include "StelProjectorType.hpp"
#include "VecMath.hpp"

#include <QObject>

class StelToneReproducer;
class StelCore;
class StelPainter;

//! @class StelSkyDrawer
//! Provide a set of methods used to draw sky objects taking into account
//! eyes adaptation, zoom level and instrument model
class StelSkyDrawer : public QObject
{
	Q_OBJECT
public:
	//! Constructor
	StelSkyDrawer(StelCore* core);
	//! Destructor
	~StelSkyDrawer();

	//! Init parameters from config file
	void init();

	//! Update with respect to the time and StelProjector/StelToneReproducer state
	//! @param deltaTime the time increment in second since last call.
	void update(double deltaTime);

	//! Set the proper openGL state before making calls to drawPointSource
	//! @param p a pointer to a valid instance of a Painter. The instance must be valid until postDrawPointSource() is called
	void preDrawPointSource(StelPainter* p);

	//! Finalize the drawing of point sources
	void postDrawPointSource(StelPainter* sPainter);

	//! Draw a point source halo.
	//! @param v the 3d position of the source in J2000 reference frame
	//! @param rcMag the radius and luminance of the source as computed by computeRCMag()
	//! @param bV the source B-V index
	//! @return true if the source was actually visible and drawn
	bool drawPointSource(StelPainter* sPainter, const Vec3d& v, const float rcMag[2], unsigned int bV, bool checkInScreen=false)
		{return drawPointSource(sPainter, v, rcMag, colorTable[bV], checkInScreen);}

	bool drawPointSource(StelPainter* sPainter,const Vec3d& v, const float rcMag[2], const Vec3f& color, bool checkInScreen=false);

	//! Terminate drawing of a 3D model, draw the halo
	//! @param v the 3d position of the source in J2000 reference frame
	//! @param illuminatedArea the illuminated area in arcmin^2
	//! @param mag the source integrated magnitude
	//! @param p the StelPainter instance to use for this drawing operation
	//! @param color the object halo RGB color
	void postDrawSky3dModel(StelPainter* sPainter, const Vec3d& v, double illuminatedArea, float mag, const Vec3f& color = Vec3f(1.f,1.f,1.f));

	//! Compute RMag and CMag from magnitude.
	//! @param mag the object integrated V magnitude
	//! @param rcMag array of 2 floats containing the radius and luminance
	//! @return false if the object is too faint to be displayed
	bool computeRCMag(float mag, float rcMag[2]) const;

	//! Report that an object of luminance lum with an on-screen area of area pixels is currently displayed
	//! This information is used to determine the world adaptation luminance
	//! This method should be called during the update operations of the main loop
	//! @param lum luminance in cd/m^2
	//! @param fastAdaptation adapt the eye quickly if true, other wise use a smooth adaptation
	void reportLuminanceInFov(double lum, bool fastAdaptation=false);

	//! To be called before the drawing stage starts
	void preDraw();

	//! Compute the luminance for an extended source with the given surface brightness
	//! @param sb surface brightness in V magnitude/arcmin^2
	//! @return the luminance in cd/m^2
	static float surfacebrightnessToLuminance(float sb);
	//! Compute the surface brightness from the luminance of an extended source
	//! @param lum luminance in cd/m^2
	//! @return surface brightness in V magnitude/arcmin^2
	static float luminanceToSurfacebrightness(float lum);

	//! Convert quantized B-V index to float B-V
	static inline float indexToBV(unsigned char bV)
	{
		return (float)bV*(4.f/127.f)-0.5f;
	}

	//! Convert quantized B-V index to RGB colors
	static inline const Vec3f& indexToColor(unsigned char bV)
	{
		return colorTable[bV];
	}

public slots:
	//! Set the way brighter stars will look bigger as the fainter ones
	void setRelativeStarScale(double b=1.0) {starRelativeScale=b;}
	//! Get the way brighter stars will look bigger as the fainter ones
	double getRelativeStarScale(void) const {return starRelativeScale;}

	//! Set the absolute star brightness scale
	void setAbsoluteStarScale(double b=1.0) {starAbsoluteScaleF=b;}
	//! Get the absolute star brightness scale
	double getAbsoluteStarScale(void) const {return starAbsoluteScaleF;}

	//! Set source twinkle amount.
	void setTwinkleAmount(double b) {twinkleAmount=b;}
	//! Get source twinkle amount.
	double getTwinkleAmount(void) const {return twinkleAmount;}

	//! Set flag for source twinkling.
	void setFlagTwinkle(bool b) {flagStarTwinkle=b;}
	//! Get flag for source twinkling.
	bool getFlagTwinkle(void) const {return flagStarTwinkle;}

	//! Set flag for displaying point sources as GLpoints (faster on some hardware but not so nice).
	void setFlagPointStar(bool b) {flagPointStar=b;}
	//! Get flag for displaying point sources as GLpoints (faster on some hardware but not so nice).
	bool getFlagPointStar(void) const {return flagPointStar;}

	//! Set the parameters so that the stars disapear at about the limit given by the bortle scale
	//! The limit is valid only at a given zoom level (around 60 deg)
	//! See http://en.wikipedia.org/wiki/Bortle_Dark-Sky_Scale
	void setBortleScale(int index);
	//! Get the current Bortle scale index
	int getBortleScale() const {return bortleScaleIndex;}

	//! Get the magnitude of the currently faintest visible point source
	//! It depends on the zoom level, on the eye adapation and on the point source rendering parameters
	//! @return the limit V mag at which a point source will be displayed
	float getLimitMagnitude() const {return limitMagnitude;}

	//! Get the luminance of the faintest visible object (e.g. RGB<0.05)
	//! It depends on the zoom level, on the eye adapation and on the point source rendering parameters
	//! @return the limit V luminance at which an object will be visible
	float getLimitLuminance() const {return limitLuminance;}

	//! Set the value of the eye adaptation flag
	void setFlagLuminanceAdaptation(bool b) {flagLuminanceAdaptation=b;}
	//! Get the current value of eye adaptation flag
	bool getFlagLuminanceAdaptation(void) const {return flagLuminanceAdaptation;}

private:
	// Debug
	float reverseComputeRCMag(float rmag) const;

	//! Compute the current limit magnitude by dichotomy
	float computeLimitMagnitude() const;

	//! Compute the current limit luminance by dichotomy
	float computeLimitLuminance() const;

	//! Get StelSkyDrawer maximum FOV.
	float getMaxAdaptFov(void) const {return maxAdaptFov;}
	//! Set StelSkyDrawer maximum FOV.
	//! Usually stars/planet halos are drawn fainter when FOV gets larger,
	//! but when FOV gets larger than this value, the stars do not become
	//! fainter any more. Must be >= 60.0.
	void setMaxAdaptFov(float fov) {maxAdaptFov = (fov < 60.f) ? 60.f : fov;}

	//! Get StelSkyDrawer minimum FOV.
	float getMinAdaptFov(void) const {return minAdaptFov;}
	//! Set StelSkyDrawer minimum FOV.
	//! Usually stars/planet halos are drawn brighter when FOV gets smaller.
	//! But when FOV gets smaller than this value, the stars do not become
	//! brighter any more. Must be <= 60.0.
	void setMinAdaptFov(float fov) {minAdaptFov = (fov > 60.f) ? 60.f : fov;}

	//! Set the scaling applied to input luminance before they are converted by the StelToneReproducer
	void setInputScale(double in) {inScale = in;}
	//! Get the scaling applied to input luminance before they are converted by the StelToneReproducer
	float getInputScale() const {return inScale;}

	//! Compute the luminance for a point source with the given mag for the current FOV
	//! @param mag V magnitude of the point source
	//! @return the luminance in log(cd/m^2)
	inline float pointSourceMagToLuminance(float mag) const {return std::exp(pointSourceMagToLnLuminance(mag));}

	//! Compute the V magnitude for a point source with the given luminance for the current FOV
	//! @param lum the luminance in cd/m^2
	//! @return V magnitude of the point source
	float pointSourceLuminanceToMag(float lum);

	//! Compute the log of the luminance for a point source with the given mag for the current FOV
	//! @param mag V magnitude of the point source
	//! @return the luminance in cd/m^2
	float pointSourceMagToLnLuminance(float mag) const;

	//! Find the world adaptation luminance to use so that a point source of magnitude mag
	//! is displayed with a halo of size targetRadius
	float findWorldLumForMag(float mag, float targetRadius);

	StelCore* core;
	StelToneReproducer* eye;

	float maxAdaptFov, minAdaptFov, lnfovFactor;
	bool flagPointStar;
	bool flagStarTwinkle;
	float twinkleAmount;

	float starRelativeScale;
	float starAbsoluteScaleF;

	float starLinearScale;	// optimization variable

	//! Current magnitude limit for point sources
	float limitMagnitude;

	//! Current magnitude luminance
	float limitLuminance;

	//! Little halo texture
	StelTextureSP texHalo;

	//! Load B-V conversion parameters from config file
	void initColorTableFromConfigFile(class QSettings* conf);

	//! Contains the list of colors matching a given B-V index
	static Vec3f colorTable[128];

	//! The current Bortle Scale index
	int bortleScaleIndex;

	//! The scaling applied to input luminance before they are converted by the StelToneReproducer
	double inScale;

	// Variables used for GL optimization when displaying point sources
	//! Buffer for storing the vertex array data
	Vec2f* verticesGrid;
	//! Buffer for storing the color array data
	Vec3f* colorGrid;
	//! Buffer for storing the texture coordinate array data
	Vec2f* textureGrid;
	//! Current number of sources stored in the buffers (still to display)
	unsigned int nbPointSources;
	//! Maximum number of sources which can be stored in the buffers
	unsigned int maxPointSources;

	//! The maximum transformed luminance to apply at the next update
	float maxLum;
	//! The previously used world luminance
	float oldLum;

	//! Big halo texture
	StelTextureSP texBigHalo;
	StelTextureSP texSunHalo;

	bool flagLuminanceAdaptation;

	bool useShader;
#if QT_VERSION>=0x040600
	class QGLShaderProgram* starsShaderProgram;
#endif
};

#endif // _STELSKYDRAWER_HPP_
