// OsuBot.h : Declares the Bot class with all its
// member functions and variables.

#pragma once

#include <Content/OsuBot/MovementModes.h>
#include <Content/OsuBot/Beatmap.h>
#include <Content/OsuBot/SigScan.h>


namespace OsuBot
{
	class Bot : public MovementModes {
	public:
		// Constructor and destructor.
		Bot(UINT targetFps, double songTimeOffset, std::wstring timeAddressSignature);
		~Bot();

		// Bot public functions (called outside OsuBot.cpp).
		void CheckGameActive(const LPVOID& main);
		std::wstring GetSongFromFolderPath();
		void AddBeatmapToQueue(const std::wstring& path);
		void UpdateSongTime();
		void AutoPlay();

	private:
		// Bot functions.
		std::wstring GetOsuFolderPath();
		void GetSongsFolderPath();

		void CheckSongActive();
		void GetCurrentSong();

		void GetTimeAddress();

	public:
		// Bot accessor functions.
		double GetSongTime() const { return m_songTime; }

		DX::Size<INT> GetOffset() const { return m_offset; }
		DX::Size<FLOAT> GetMultiplier() const { return m_multiplier; }

		const BeatmapInfo::Beatmap* GetBeatmapAtIndex(const UINT& index) const { return &m_beatmapQueue.at(index); }


	public:
		// Public bot variables.
		HWND m_targetHwnd;
		bool m_beatmapAuto;
		bool m_sigFound;
		bool m_autoClickPressed;
		std::wstring m_songName;
		UINT m_selectedBeatmapIndex;
		UINT m_hitObjectIndex;
		POINT m_cursorPosition;
		std::vector<BeatmapInfo::Beatmap> m_beatmapQueue;


	private:
		// Bot variables.
		HANDLE m_gameProcessHandle;
		std::wstring m_gameTitle;
		UINT m_timerFrameCount;
		UINT m_targetFps;
		RECT m_targetRect;
		INPUT m_input;
		double m_prevSongTime;
		double m_songTime;
		double m_songTimeOffset;
		bool m_songStarted;
		bool m_songPaused;
		bool m_beatmapFinished;

		// Beatmap song variables.
		std::wstring m_currentSongName;
		std::wstring m_currentSongVersion;
		std::wstring m_songsFolderPath;
		std::vector<std::wstring> m_BeatmapSetNames;

		// Movement variables.
		DX::Size<INT> m_offset;
		DX::Size<FLOAT> m_multiplier;
		float m_movementAmplifier;
		bool m_hardrock;

		// Time variables.
		std::unique_ptr<DWORD> m_timeAddress;


	public:
		// Bot logic loop timer.
		DX::StepTimer m_logicTimer;

	private:
		// SigScanner for time address.
		std::wstring m_timeAddressSignature;
		SigScan::SigScanner m_sigScanner;
	};
}