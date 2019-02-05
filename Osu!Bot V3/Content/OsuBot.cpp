// OsuBot.cpp : Defines the constructor, destructor
// and all member functions for the Bot class.

#include <Common/Pch.h>

#include <Content/OsuBot.h>
#include <Content/AppMain.h>


using namespace OsuBot;


// Constructor of the Bot with Initialiazion code.
Bot::Bot(UINT targetFps, double songTimeOffset, std::wstring timeAddressSignature) :
	m_gameProcessHandle(nullptr),
	m_gameTitle(L""),
	m_songName(L"Idle"),
	m_targetFps(targetFps),
	m_targetHwnd(NULL),
	m_sigFound(FALSE),
	m_songStarted(FALSE),
	m_songPaused(FALSE),
	m_songTimeOffset(songTimeOffset),
	m_prevSongTime(0.0),
	m_offset(0, 0),
	m_multiplier(0.f, 0.f),
	m_hitObjectIndex(0U),
	m_movementAmplifier(1.f),
	m_beatmapAuto(FALSE),
	m_hardrock(FALSE),
	m_autoClickPressed(FALSE),
	m_selectedBeatmapIndex(UINT_MAX),
	m_timeAddressSignature(timeAddressSignature),
	m_timeAddress(nullptr)
{
	// Set fixed timestep update logic.
	m_logicTimer.SetFixedTimeStep(TRUE);
	m_logicTimer.SetTargetElapsedSeconds(1.0 / (DOUBLE)m_targetFps);

	// Initialize the input.
	m_input.type = INPUT_MOUSE;
	ZeroMemory(&m_input, sizeof(m_input));

	// Set the movement variables.
	m_movementModeCircle = MODE_PREDICTING;
	m_movementModeSlider = MODE_STANDARD;
	m_movementModeSpinner = MODE_STANDARD;
	m_spinnerRadius = 450.f;
}

// Destructor of the Bot class.
Bot::~Bot() {
	m_targetHwnd = NULL;
}


// This function should only be used to check if the game is running or not.
// When the game is active (running) it updates the screen metrics if the 
// target rect changed. And sets m_targetHwnd to a valid HWND.
void Bot::CheckGameActive(const LPVOID& main) {
	if (m_targetHwnd == NULL) {
		// Get the HWND if not yet set.
		m_targetHwnd = FindWindowW(NULL, L"osu!");
	}
	else if (m_logicTimer.GetFrameCount() == m_timerFrameCount) {
		// Only continue the function if the logic timer has made a tick.
		return;
	}
	else {
		// Update the elapsed frame count.
		m_timerFrameCount = m_logicTimer.GetFrameCount();

		WCHAR buffer[MAX_LOADSTRING];

		// Check if the game is still running.
		GetWindowTextW(m_targetHwnd, buffer, MAX_LOADSTRING);
		m_gameTitle.assign(buffer);
		if (m_gameTitle == L"" && !m_songStarted) {
			m_targetHwnd = FindWindowW(NULL, L"osu!");
			if (m_targetHwnd == NULL) {
				// The game has exited.
				m_targetHwnd = NULL;
				m_sigFound = FALSE;
			}
		}
		else {
			if ((*reinterpret_cast<const std::shared_ptr<OsuBot::AppMain>*>(main))->m_deviceResources->m_drawing == FALSE) {
				(*reinterpret_cast<const std::shared_ptr<OsuBot::AppMain>*>(main))->m_deviceResources->m_resizeing = TRUE;

				// Get the current working rect.
				(*reinterpret_cast<const std::shared_ptr<OsuBot::AppMain>*>(main))->GetWorkingRect();

				(*reinterpret_cast<const std::shared_ptr<OsuBot::AppMain>*>(main))->m_deviceResources->m_resizeing = FALSE;
			}

			RECT rect;
			CopyRect(&rect, &((*reinterpret_cast<const std::shared_ptr<OsuBot::AppMain>*>(main))->m_rect));

			// Check if the working rect changed.
			if (!EqualRect(&rect, &m_targetRect)) {
				// Get the new screen metrics.
				RECT clientRect;
				POINT w = { 0, 6 };
				GetClientRect(m_targetHwnd, &clientRect);
				ClientToScreen(m_targetHwnd, &w);

				int x = min(clientRect.right, GetSystemMetrics(SM_CXSCREEN));
				int	y = min(clientRect.bottom, GetSystemMetrics(SM_CXSCREEN));

				// Get x,y multipliers for the movement calculations.
				int sWidth = x;
				int sHeight = y;

				if (sWidth * 3 > sHeight * 4) {
					sWidth = sHeight * 4 / 3;
				}
				else {
					sHeight = sWidth * 3 / 4;
				}

				m_multiplier.Width = sWidth / 640.f;
				m_multiplier.Height = sHeight / 480.f;

				// Get the x,y offsets for the movement calculations.
				int xOffset = (INT)floorf(x - 512.f * m_multiplier.Width) / 2;
				int yOffset = (INT)floorf(y - 384.f * m_multiplier.Height) / 2;

				m_offset.Width = w.x + xOffset;
				m_offset.Height = w.y + yOffset;

				CopyRect(&m_targetRect, &rect);
			}
		}
	}
}

// This function should be called every time the bot logic updates.
// To check whether the song is playing, paused, or stopped.
void Bot::CheckSongActive() {
	// Check if the songs folder has been assigned.
	if (m_songsFolderPath != L"") {
		if (m_gameTitle != L"osu!" && m_gameTitle != L"") {
			// Check if the song has been paused.
			if (m_songStarted && GetSongTime() == m_prevSongTime) {
				m_songPaused = TRUE;

				ClipCursor(nullptr);
			}
			else {
				m_songPaused = FALSE;

				ClipCursor(&m_targetRect);
			}

			// Get the current beatmap name and difficulty.
			GetCurrentSong();

			if (m_beatmapAuto) {
				// Find the beatmap set with the beatmap name.
				// TODO: Implement auto beatmap search function.
			}
			else {
				// Check for beatmaps in the queue.
				if (m_beatmapQueue.empty()) {
					// No beatmaps queued don't start the autoplay.
					m_songStarted = FALSE;

					// Send notification to user that no beatmaps were queued.
					static bool warning = FALSE;
					if (!warning) {
						OutputDebugStringW(L"WARNING : No beatmaps queued to select the currently playing song from.\n");
						warning = TRUE;
					}
				}
				else {
					// Check any queued map matches current selected map.
					for (UINT i = 0U; i < m_beatmapQueue.size(); i++) {
						m_songName = (GetBeatmapAtIndex(i)->GetArtist() + L" - " + GetBeatmapAtIndex(i)->GetTitle());

						if (m_songName == m_currentSongName) {
							m_songStarted = TRUE;
							m_selectedBeatmapIndex = i;

							ClipCursor(&m_targetRect);
							break;
						}
						else {
							m_songName = L"Idle";
						}
					}
				}
			}
		}
		else if (m_songStarted && m_songName != L"Idle") {
			// Not playing a beatmap anymore. Reset the current state.
			m_songStarted = FALSE;
			m_songPaused = FALSE;
			m_beatmapFinished = TRUE;

			m_hitObjectIndex = 0U;
			m_songName = L"Idle";

			ClipCursor(nullptr);
		}
		else if (!m_beatmapQueue.empty() && m_beatmapFinished && m_selectedBeatmapIndex != UINT_MAX) {
			try {
				// Remove the beatmap from the queue.
				m_beatmapQueue.erase(m_beatmapQueue.begin() + m_selectedBeatmapIndex);

				m_beatmapFinished = FALSE;
			}
			catch (...) {
				// Oops something when wrong while trying to delete a beatmap from the queue.
				// TODO: Throw error if needed.

			}
			m_selectedBeatmapIndex = UINT_MAX;
		}
	}
	else {
		// The songs folder was not assigned, get the folder from the osu!.exe.
		GetSongsFolderPath();
	}
}

// Add a beatmap the the queue if it parsed.
void Bot::AddBeatmapToQueue(const std::wstring& path) {
	try {
		// Create a beatmap and assign the file.
		BeatmapInfo::Beatmap beatmap(path.c_str());

		// Parse the beatmap.
		if (beatmap.ParseBeatmap()) {
			// On success, add it to the queue.
			m_beatmapQueue.push_back(beatmap);
		}
	}
	catch (...) {
		// Oops something when wrong with creating/parsing a beatmap.
		// TODO:  Thow error if needed.
		
		OutputDebugStringW(L"ERROR : Beatmap could not be queued.\n");
	}
}


// This function handles the autoplay feature of OsuBot.
void Bot::AutoPlay() {
	// Don't make unnecessary updates.
	m_logicTimer.Tick([&]() {
		// Update the song time.
		UpdateSongTime();

		// Check if the song is playing.
		CheckSongActive();

		if (m_songStarted && !m_songPaused && m_hitObjectIndex <= GetBeatmapAtIndex(m_selectedBeatmapIndex)->GetHitObjectsCount()) {
			// Get the current hit object from the current beatmap in the queue.
			const BeatmapInfo::HitObject* currentObject = GetBeatmapAtIndex(m_selectedBeatmapIndex)->GetHitObjectAtIndex(m_hitObjectIndex);
			if (currentObject == nullptr) {
				// The hitobject was not set, return to prevent read access exceptions.
				return;
			}

			// Song has started playing, call movement functions.
			if (currentObject->GetStartTime() > GetSongTime()) {
				switch (m_movementModeCircle) {
				case MODE_NONE:
					break;

				case MODE_STANDARD:
					MoveToObject(this, &MovementModes::ControlPointStandard);
					break;

				case MODE_FLOWING:
					MoveToObject(this, &MovementModes::ControlPointFlowing);
					break;

				case MODE_PREDICTING:
					MoveToObject(this, &MovementModes::ControlPointPredicting);
					break;
				}
			}

			if (currentObject->GetObjectType() == HITOBJECT_SLIDER && currentObject->GetStartTime() < GetSongTime()) {
				switch (m_movementModeSlider) {
				case MODE_NONE:
					break;

				case MODE_STANDARD:
					MovementSlider(this, &MovementModes::ControlPointStandard);
					break;

				case MODE_FLOWING:
					MovementSlider(this, &MovementModes::ControlPointFlowing);
					break;

				case MODE_PREDICTING:
					MovementSlider(this, &MovementModes::ControlPointPredicting);
					break;
				}
			}
			else if (currentObject->GetObjectType() == HITOBJECT_SPINNER && currentObject->GetStartTime() < GetSongTime()) {
				switch (m_movementModeSpinner) {
				case MODE_NONE:
					break;

				case MODE_STANDARD:
					MovementSpinner(this, &MovementModes::ControlPointStandard);
					break;

				case MODE_FLOWING:
					MovementSpinner(this, &MovementModes::ControlPointFlowing);
					break;

				case MODE_PREDICTING:
					MovementSpinner(this, &MovementModes::ControlPointPredicting);
					break;
				}
			}

			if (currentObject->GetStartTime() <= GetSongTime()) {
				if (!m_autoClickPressed) {
					// Press.
					m_input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
					SendInput(1U, &m_input, sizeof(m_input));
					//OutputDebugStringW((L"Pressed - " + std::to_wstring(m_hitObjectIndex)).c_str());

					m_autoClickPressed = TRUE;
				}
				if (currentObject->GetEndTime() <= GetSongTime()) {
					// Release.
					m_input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
					SendInput(1U, &m_input, sizeof(m_input));
					//OutputDebugStringW(L" - Released\n");

					m_autoClickPressed = FALSE;

					if (m_bezierPts.size() >= 2U) {
						// Clear the bezier vector.
						m_bezierPts.clear();
					}

					// Add one to the hitObject index.
					m_hitObjectIndex++;
				}
			}
		}
	});
}


// This function is used to get the currently playing song time.
void Bot::UpdateSongTime() {
	// Check if time address is set.
	if (!m_timeAddress.get()) {
		// Find the time address.
		GetTimeAddress();
	}

	// Store the song time into prev song time.
	m_prevSongTime = m_songTime;

	// Read the new song time from memory to m_songTime.
	ReadProcessMemory(m_gameProcessHandle, reinterpret_cast<LPVOID>(*m_timeAddress.get()), &m_songTime, sizeof(DOUBLE), nullptr);

	// Offset the songtime for better timing accuracy.
	m_songTime += m_songTimeOffset;
}

// This function is used to get the time address of the game.
void Bot::GetTimeAddress() {
	// First find the process.
	if (m_sigScanner.GetProcess(L"osu!.exe")) {
		// Then set store the process handle in m_gameProcessHandle.
		m_gameProcessHandle = m_sigScanner.GetTargetProcessHandle();

		// Now find the signature in the process memory space.
		m_sigScanner.FindSignature(&m_timeAddressSignature);

		// Set flag to notify the user of status.
		m_sigFound = m_sigScanner.SigFound();

		// Continue if the signature was found.
		if (m_sigFound) {
			DWORD sigAddress = m_sigScanner.GetResultAddress();

			// Offset the sig to the timeAddress.
			sigAddress -= 0xA;

			// Get the time address from the result.
			DWORD resultAddress;
			ReadProcessMemory(m_gameProcessHandle, reinterpret_cast<LPCVOID>(sigAddress), &resultAddress, 4UL, nullptr);

			// Store the resulting address in m_timeAddress.
			m_timeAddress = std::make_unique<DWORD>(resultAddress);
		}
	}
}