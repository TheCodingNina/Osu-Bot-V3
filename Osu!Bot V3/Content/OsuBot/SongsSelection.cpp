#include <Common/Pch.h>

#include <Content/OsuBot.h>

#include <TlHelp32.h>

#include <ShObjIdl.h>


using namespace OsuBot;


// Gets the currently playing song name and difficulty from the game title.
void Bot::GetCurrentSong() {
	UINT bracketCount = 0U;
	UINT hit = NULL;

	// Get the version name from the game title.
	for (UINT i = (UINT)m_gameTitle.size(); i-- > 0U;) {
		if (m_gameTitle.at(i) == L']') {
			// Increase the bracket count by 1.
			bracketCount++;

			// Mark the bracket index. (closing bracket)
			if (hit == NULL) {
				hit = i;
			}
		}
		else if (m_gameTitle.at(i) == L'[') {
			// Subtract 1 from the bracket count.
			bracketCount--;
		}

		// Check if the amount of opening brackets equal the closing brackets.
		if (bracketCount == 0U && hit != NULL) {
			// Set vesion string without the outer brackets.
			m_currentSongVersion = m_gameTitle.substr(i + 1U, hit - i - 1U);
			break;
		}
	}


	// Get the beatmap name from the game title to the index of version.
	// Beatmap name has format "{artistName} - {songTitle}" (without the parentheses).
	m_currentSongName = m_gameTitle.substr(8U, m_gameTitle.size() - m_currentSongVersion.size() - 11U);

	// Pop the last character back, if it is a space.
	if (m_currentSongName.back() == L' ') {
		m_currentSongName.pop_back();
	}
}


// Get the osu songs folder path.
void Bot::GetSongsFolderPath() {
	// Get the osu folder path.
	std::wstring path = GetOsuFolderPath();

	// Truncate the "osu!.exe" part.
	path = path.substr(0U, path.find_last_of(L'\\') + 1U);

	// Append "Songs".
	path.append(L"Songs");

	// Assign the songs folder path.
	m_songsFolderPath = path;
}

// Retrives the full path to osu!.exe.
std::wstring Bot::GetOsuFolderPath() {
	// Get a handle to a module.
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_sigScanner.GetTargetProcessID());

	MODULEENTRY32W mEntry;
	mEntry.dwSize = sizeof(mEntry);

	// Find the process name with a MODULEENTRY32W object.
	do {
		if (std::wstring(mEntry.szModule) == L"osu!.exe") {
			// Name found, close the handle and return path.
			CloseHandle(hModule);
			return mEntry.szExePath;
		}
	} while (Module32NextW(hModule, &mEntry));

	// No module found with the name.
	// Close the handle and return empty string.
	CloseHandle(hModule);
	return NULL;
}


// This function querries the user to select a beatmap to append to the beatmap queue.
std::wstring Bot::GetSongFromFolderPath() {
	std::wstring result;
	IFileOpenDialog* pFileDialog;

	// Create the FileOpenDialog object.
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));

	if (SUCCEEDED(hr)) {
		DWORD dwOptions;
		std::vector<COMDLG_FILTERSPEC> filterSpec = {
			{ L"osu! beatmap", L"*.osu" }
		};

		// Force the folder to be set to the songs folder.
		IShellItem* pSongsFolder;
		SHCreateItemFromParsingName(m_songsFolderPath.data(), NULL, IID_PPV_ARGS(&pSongsFolder));

		// Get the options.
		pFileDialog->GetOptions(&dwOptions);

		// Configure the options.
		// Only allow filtered files to be selected.
		pFileDialog->SetFolder(pSongsFolder);
		pFileDialog->SetOptions(dwOptions | FOS_STRICTFILETYPES);
		pFileDialog->SetFileTypes((UINT)filterSpec.size(), filterSpec.data());

		// Show the dialog.
		hr = pFileDialog->Show(NULL);

		if (SUCCEEDED(hr)) {
			IShellItem* pItem;

			// Get the result.
			hr = pFileDialog->GetResult(&pItem);

			if (SUCCEEDED(hr)) {
				LPWSTR lpName;

				// Get the file name from the result.
				hr = pItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &lpName);

				// Store the display name in the result string.
				if (SUCCEEDED(hr)) {
					result = lpName;
				}

				// Function is done, Release the resources.
				CoTaskMemFree(lpName);
				pItem->Release();
			}
			pSongsFolder->Release();
		}
		pFileDialog->Release();
	}

	return result;
}