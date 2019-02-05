// ConfigurationIni.h : Defines a class responsible for read and writing
// from and to a configuration ini file.

#pragma once

#include <Common/Pch.h>


namespace ConfigurationIni
{
	class ConfigIni {
	public:
		// Constructor and destructor.
		ConfigIni() :
			m_configFilePath(L"Config.ini")
		{
			// Open file for reading.
			configFile = std::unique_ptr<FILE>(_wfsopen(m_configFilePath, L"r", _SH_DENYNO));
		}
		~ConfigIni() {
			fclose(configFile.get());
			configFile.reset();
		}


	public:
		// Configuration Enumuration and array of strings.
		enum configSections {
			time,
			configuration,
			count
		};

	private:
		const wchar_t* configStrings[configSections::count] = {
			L"[TIME]",
			L"[CONFIGURATION]"
		};


		// Configuration ini functions.
	public:
		template<typename _T>
		BOOL _stdcall ReadFromConfigFile(
			_In_ configSections configSection,
			_In_ LPCWSTR lpConfigKey,
			_In_ _T* lpResultValue,
			_In_opt_ int maxLenght = MAX_READSTRING,
			_In_opt_ _T lpDefaultValue = NULL
		) {
			std::wstring readLine;
			LPWSTR lpReadLine = new wchar_t[maxLenght];

			// Get configuration string from the array with the enum value.
			LPCWSTR lpConfigSection = configStrings[configSection];

			// Read the file.
			if (configFile.get() != nullptr) {
				// Set the read position to the begin of the file.
				fpos_t pos = 0;
				fsetpos(configFile.get(), &pos);

				// Find the configuration header in the file.
				while (TRUE) {
					if (feof(configFile.get())) {
						// End of file, return FALSE with the default value as result.
						*lpResultValue = lpDefaultValue;

						delete lpReadLine;
						return FALSE;
					}
					else if (readLine.find(lpConfigSection) == std::wstring::npos) {
						// Header not found, read next line.
						fgetws(lpReadLine, maxLenght, configFile.get());
						readLine.assign(lpReadLine);
					}
					else {
						// Header found, read the value from the key.
						if (ReadConfigKeyValue(lpConfigKey, lpResultValue, maxLenght)) {
							// return TRUE when succeeded with the value as result.
							delete lpReadLine;
							return TRUE;
						}
					}
				}
			}

			// File invalid, return FALSE with the default value as result.
			*lpResultValue = lpDefaultValue;
			delete lpReadLine;
			return FALSE;
		}

	private:
		template<typename _T>
		BOOL _stdcall ReadConfigKeyValue(
			_In_ LPCWSTR lpConfigKey,
			_In_ _T lpResultValue,
			_In_ int maxLenght
		) {
			std::wstring readLine;
			LPWSTR lpReadLine = new wchar_t[maxLenght];
			const std::wstring numbers = L"0123456789";

			// Find the configuration key in the file.
			while (TRUE) {
				if (feof(configFile.get())) {
					// End of file, return FALSE.
					delete lpReadLine;
					return FALSE;
				}
				else if (readLine.find(lpConfigKey) == std::wstring::npos) {
					// Key not found, read the next line.
					fgetws(lpReadLine, maxLenght, configFile.get());
					readLine.assign(lpReadLine);
				}
				else if (readLine.find(m_comment) != std::wstring::npos) {
					// Comment found, read the next line.
					fgetws(lpReadLine, maxLenght, configFile.get());
					readLine.assign(lpReadLine);
				}
				else {
					// Key is found.
					if (readLine.substr(0U, readLine.find(m_delimiter)) == lpConfigKey) {
						// Key is exactly the same, get the value.
						UINT valuePos = (UINT)readLine.find_first_of(m_delimiter) + 1U;

						if (valuePos == (UINT)readLine.size()) {
							// Pos is equal to the string size, return FALSE with no value.
							delete lpReadLine;
							return FALSE;
						}


						if (readLine.at(valuePos) == L'-') {
							// Value is negative.
							valuePos += 1U;
						}

						// Set the value.
						if (numbers.find(readLine.at(valuePos)) != std::wstring::npos) {
							// The character at valuePos is a number.
							if (readLine.find(L'.') != std::wstring::npos) {
								// Value is has a decimal.
								*reinterpret_cast<DOUBLE*>(lpResultValue) = std::stod(readLine.substr(valuePos));
							}
							else {
								// Value is of type UINT.
								*reinterpret_cast<UINT*>(lpResultValue) = std::stoi(readLine.substr(valuePos));
							}
						}
						else {
							// Return the wstring.
							*reinterpret_cast<std::wstring*>(lpResultValue) = readLine.substr(valuePos).data();
						}


						// Value set, return TRUE.
						delete lpReadLine;
						return TRUE;
					}
				}
			}
		}


	private:
		// Configuration ini variables.
		std::unique_ptr<FILE> configFile;
		static const wchar_t m_delimiter = L'=';
		static const wchar_t m_comment = L'#';

	public:
		LPCWSTR m_configFilePath;
	};
}