// AppMain.h : Declares the main app class and holds
// all the content of the app as members of itself.

#pragma once

// TODO: Include app content.
#include <Content/OsuBot.h>
#include <Content/UI Elements/StaticText.h>

#include <Common/ConfigurationIni.h>
#include <Common/DeviceResources.h>


namespace OsuBot
{
	class AppMain : public DX::IDeviceNotify {
	public:
		// Constructor and destructor.
		AppMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~AppMain();

		// App functions.
		WORD MyRegisterClass(HINSTANCE hInstance, WNDPROC windowProc);
		bool InitInstance(HINSTANCE hInstance, int nCmdShow);
		bool GetWorkingRect();
		void InitContent();
		void ReadConfigSettings();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Draw();

	private:
		// IDeviceNotify.
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();


	public:
		// Public app variables.
		HWND m_hWnd;
		RECT m_rect;
		WCHAR m_windowClass[MAX_LOADSTRING];
		WCHAR m_title[MAX_LOADSTRING];
		DX::Size<FLOAT> m_windowSize;
		bool m_debugInfoVisible;
		bool m_hudVisible;
		bool m_quit;

	private:
		// App variables.
		double m_songTimeOffset;
		UINT m_windowFps;
		UINT m_targetFps;
		UINT m_timerFrameCount;
		HINSTANCE m_hInstance;
		COLORREF m_windowTransparencyColor;
		BYTE m_windowTransparencyAlpha;
		std::wstring m_timeAddressSignature;


	public:
		// Cached pointer to the device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Cached pointer to the bot.
		std::unique_ptr<OsuBot::Bot> m_osuBot;

	private:
		// TODO: place app content pointer here.
		std::unique_ptr<UIElements::StaticText> m_songNameRenderer; 
		std::unique_ptr<UIElements::StaticText> m_beatmapQueueNamesRenderer;

		std::unique_ptr<UIElements::StaticText> m_timeRenderer;
		std::unique_ptr<UIElements::StaticText> m_fpsRenderer;

		std::unique_ptr<ConfigurationIni::ConfigIni> m_configIni;

	public:
		// Drawing loop timer.
		DX::StepTimer m_timer;
	};
}