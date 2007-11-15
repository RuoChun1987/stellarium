/*
* Stellarium
* Copyright (C) 2005 Fabien Chereau
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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <config.h>

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "StelUtils.hpp"
#include <QString>

// These macro are used as global function replacing standard gettext operation
#include "gettext.h"
#define _(String) Translator::globalTranslator.translate( gettext_noop(String) ).c_str()
#define q_(String) Translator::globalTranslator.qtranslate( gettext_noop(String) )
#define N_(String) gettext_noop(String)

using namespace std;

//! Class used to translate strings to any language.
//! Implements a nice interface to gettext which is UTF-8 compliant and is somewhat multiplateform
//! All its operations do not modify the global locale.
//! The purpose of this class is to remove all non-OO C locale functions from stellarium.
//! @author Fabien Chereau
class Translator
{
public:

	//! Create a translator from a language name.
	//! If the passed locale name cannot be handled by the system, default value will be used.
	//! The passed language name is a language code string like "fr" or "fr_FR".
	//! This class wrap gettext to simulate an object oriented multiplateform gettext UTF8 translator
	//! @param _domain The name of the domain to use for translation
	//! @param _moDirectory The directory where to look for the domain.mo translation files.
	//! @param _langName The C locale name or language name like "fr" or "fr_FR". If string is "" or "system" it will use the system locale.
	Translator(const std::string& _domain, const QString& _moDirectory, const QString& _langName) :
			domain(_domain), moDirectory(_moDirectory), langName(_langName)
	{
		Translator::lastUsed = NULL;
	}

	//! Translate input message.
	//! @param s input string in english.
	//! @return The translated string in UTF-8 characters.
	std::string translateUTF8(const std::string& s)
	{
		reload();
		return gettext(s.c_str());
	}

	//! Translate input message.
	//! @param s input string in english.
	//! @return The translated string in wide characters.
	std::wstring translate(const std::string& s)
	{
		if (s.empty()) return L"";
		return StelUtils::stringToWstring(translateUTF8(s));
	}
	
	//! Translate input message and return it as a QString.
	//! @param s input string in english.
	//! @return The translated QString
	QString qtranslate(const std::string& s)
	{
		if (s.empty()) return QString();
		reload();
		return QString::fromUtf8(gettext(s.c_str()));
	}
	
	//! Get true translator locale name. Actual locale, never "system".
	//! @return Locale name e.g "fr_FR"
	const QString& getTrueLocaleName(void) const
	{
		if (langName=="system" || langName=="system_default") {
			return Translator::systemLangName;
		} else {
			return langName;
		}
	}

	//! Used as a global translator by the whole app
	static Translator globalTranslator;

	//! Get available language name in native language from passed locales directory
	static std::wstring getAvailableLanguagesNamesNative(const QString& localeDir);	
	
	//! Get available language codes from passed locales directory
	static std::vector<QString> getAvailableLanguagesIso639_1Codes(const QString& localeDir);

	//! Convert from ISO639-1 2 letters langage code to native language name
	static std::wstring iso639_1LanguageCodeToNativeName(const QString& languageCode);
	
	//! Convert from native language name to ISO639-1 2 letters langage code 
	static QString nativeLanguageNameCodeToIso639_1(const wstring& languageName);
	
	//! Try to determine system language from system configuration
	static void initSystemLanguage(void);

	//! Initialize the languages code list from the passed file
	//! @param fileName file containing the list of language codes
	static void initIso639_1LanguageCodes(const QString& fileName);
	
private:
	//! Reload the current locale info so that gettext use them
	void reload();

	//! The domain name
	string domain;

	//! The directory where the locale file tree stands
	QString moDirectory;

	//! The two letter string defining the current language name
	QString langName;

	//! Keep in memory which one was the last used transator to prevent reloading it at each tranlate() call
	static Translator* lastUsed;

	//! Store the system default language name as taken from LANGUAGE environement variable
	static QString systemLangName;
	
	//! Contains the list of all iso639 languages codes
	static map<QString, wstring> iso639codes;
};

#endif
