/*
 * Stellarium Scenery3d Plug-in
 *
 * Copyright (C) 2011 Simon Parzer, Peter Neubauer, Georg Zotti, Andrei Borza
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
 
attribute vec4 vecTangent;

varying vec3 vecLight;
varying vec3 vecHalf;

void main(void)
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	
	vec3 n = normalize(gl_NormalMatrix * gl_Normal);
	vec3 t = normalize(gl_NormalMatrix * vecTangent.xyz);
	vec3 b = cross(n, t) * vecTangent.w;
	
	mat3 tbnv = mat3(t.x, b.x, n.x,
					 t.y, b.y, n.y,
					 t.z, b.z, n.z);
	
	vecLight = gl_LightSource[0].position.xyz;
	vecLight = tbnv * vecLight;
	
	vecHalf = gl_LightSource[0].halfVector.xyz;
	vecHalf = tbnv * vecHalf; 
}

