// AppMain.cpp : Holds the main app class definitions.

#include <Common/Pch.h>

#include <Content/AppMain.h>


using namespace OsuBot;


// Constructor of the app class.
// With initialiaztion code.
AppMain::AppMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources),
	m_windowTransparencyColor(RGB(0x00, 0x00, 0x00)),
	m_windowTransparencyAlpha(0xff),
	m_windowFps(60U),
	m_hInstance(NULL),
	m_hudVisible(TRUE),
	m_debugInfoVisible(FALSE),
	m_quit(FALSE) {
	// Assign the device resources.
	m_deviceResources->RegisterDeviceNotify(this);

	// Read the config ini for defined app settings.
	ReadConfigSettings();

	// Set fixed timestep update logic.
	m_timer.SetFixedTimeStep(TRUE);
	m_timer.SetTargetElapsedSeconds(1.0 / (DOUBLE)m_windowFps);
}

// Destructor of the app class.
AppMain::~AppMain() {
	// Unassign the device resources.
	m_deviceResources->RegisterDeviceNotify(nullptr);
	m_deviceResources->m_main = nullptr;
}


// Initializes the app contents.
void AppMain::InitContent() {
	// Assign a pointer to the app to the device resources.
	m_deviceResources->SetMainWindow(this);

	// TODO: place app content initializers here.
	m_osuBot = std::make_unique<OsuBot::Bot>(
		m_targetFps,
		m_songTimeOffset,
		m_timeAddressSignature
		);

	m_songNameRenderer = std::make_unique<UIElements::StaticText>(
		m_deviceResources,
		m_windowTransparencyAlpha,
		D2D1::ColorF::Red,
		DX::Size<FLOAT>(800.f, 20.f),
		24.f,
		L"",
		TRUE,
		RECT { 0, 0, 804, 64 },
		D2D1::ColorF::Gray,
		DWRITE_TEXT_ALIGNMENT_LEADING
		);

	m_beatmapQueueNamesRenderer = std::make_unique<UIElements::StaticText>(
		m_deviceResources,
		m_windowTransparencyAlpha,
		D2D1::ColorF::LightSkyBlue,
		DX::Size<FLOAT>(300.f, 600.f),
		16.f,
		L"",
		TRUE,
		RECT { 0, 0, 304, 604 },
		D2D1::ColorF::Gray,
		DWRITE_TEXT_ALIGNMENT_LEADING
		);


	// Debug renderers.
	m_timeRenderer = std::make_unique<UIElements::StaticText>(
		m_deviceResources,
		m_windowTransparencyAlpha,
		D2D1::ColorF::YellowGreen,
		DX::Size<FLOAT>(260.f, 10.f),
		20.f,
		L"",
		TRUE,
		RECT { 0, 0, 264, 24 },
		D2D1::ColorF::Blue
		);

	m_fpsRenderer = std::make_unique<UIElements::StaticText>(
		m_deviceResources,
		m_windowTransparencyAlpha,
		D2D1::ColorF::YellowGreen,
		DX::Size<FLOAT>(140.f, 10.f),
		32.f,
		L"",
		TRUE,
		RECT { 0, 0, 144, 36 },
		D2D1::ColorF::Blue
		);
}


// Read the values from the config ini for the app.
void AppMain::ReadConfigSettings() {
	m_configIni = std::make_unique<ConfigurationIni::ConfigIni>();

	// TODO: add aditional variables that need to be read from the config ini.
	m_configIni->ReadFromConfigFile<std::wstring>(m_configIni->time, L"TIME_SIGNATURE", &m_timeAddressSignature, MAX_READSTRING, std::wstring());
	m_configIni->ReadFromConfigFile<DOUBLE>(m_configIni->time, L"SONG_OFFSET", &m_songTimeOffset, MAX_READSTRING, DOUBLE(0.0));

	m_configIni->ReadFromConfigFile<UINT>(m_configIni->configuration, L"WINDOW_FPS", &m_windowFps, MAX_READSTRING, (UINT)60U);
	m_configIni->ReadFromConfigFile<UINT>(m_configIni->configuration, L"LOGIC_UPS", &m_targetFps, MAX_READSTRING, (UINT)60U);
	m_configIni->ReadFromConfigFile<COLORREF>(m_configIni->configuration, L"TRANSPARENCY_COLOR", &m_windowTransparencyColor, MAX_READSTRING, (COLORREF)0);
	m_configIni->ReadFromConfigFile<BYTE>(m_configIni->configuration, L"TRANSPARENCY_ALPHA", &m_windowTransparencyAlpha, MAX_READSTRING, (BYTE)0xff);

	m_configIni.release();
}


// Update the app state when the window size changes.
void AppMain::CreateWindowSizeDependentResources() {
	// TODO: place size-dependent initialization functions of app contents.

}

// Updates the app content once per rendered frame.
void AppMain::Update() {
	// Update the scene.
	m_timer.Tick([&]() {
		// Only execute if HUD is active.
		// Check this here otherwise timer tries to catch up with the fixed fps.
		if (!m_hudVisible) {
			return;
		}

		// TODO: place app content update functions here.
		m_songNameRenderer->SetTranslation(DX::Size<FLOAT>(3.f, 1.f));
		if (m_osuBot->m_songName != L"Idle") {
			m_songNameRenderer->Update(L"Now playing : " + m_osuBot->m_songName);
		}
		else {
			m_songNameRenderer->Update(L"Idle");
		}

		m_beatmapQueueNamesRenderer->SetTranslation(DX::Size<FLOAT>(3.f, 80.f));
		std::wstring names = L"Beatmap Queue (Insert to add)\n-----------------------------\n";
		for (auto beatmap : m_osuBot->m_beatmapQueue) {
			names += beatmap.GetTitle();
			names += L"\n";
		}
		m_beatmapQueueNamesRenderer->Update(names);


		// Update debug information (normaly not used).
		if (m_debugInfoVisible) {
			// Current song time.
			std::wstring time;
			if (m_osuBot->m_sigFound) {
				time = std::to_wstring(std::trunc(m_osuBot->GetSongTime()) / 1000);
				time = time.substr(0U, time.length() - 3U).append(L"s");
			}
			else {
				time = L"Signature not found!";
			}

			m_timeRenderer->SetTranslation(DX::Size<FLOAT>(3.f, m_deviceResources->GetLogicalSize().Height - 27.f));
			m_timeRenderer->Update(time);

			// Current fps.
			UINT fps = m_osuBot->m_logicTimer.GetFramesPerSecond();
			std::wstring fpsString = (fps > 0U) ? std::to_wstring(fps) + L" FPS" : L" - FPS";

			m_fpsRenderer->SetTranslation(m_deviceResources->GetLogicalSize() - DX::Size<FLOAT>(147.f, 39.f));
			m_fpsRenderer->Update(fpsString);
		}
	});
}

// Draw the current frame according to the current app content.
// Returns true if the frame was drawn and is ready.
// Returns false if the render target needed to be recreated.
bool AppMain::Draw() {
	// Don't draw anything before any updated scenes.
	if (!m_hudVisible || m_timer.GetFrameCount() == m_timerFrameCount) {
		return FALSE;
	}
	m_timerFrameCount = m_timer.GetFrameCount();

	if (m_deviceResources->GetD2DRenderTarget() != nullptr || m_deviceResources->m_resizeing == FALSE) {
		try {
			m_deviceResources->m_drawing = TRUE;

			// Clear the previous scene.
			m_deviceResources->GetD2DRenderTarget()->BeginDraw();
			m_deviceResources->GetD2DRenderTarget()->Clear(D2D1::ColorF(D2D1::ColorF::Black));

			// TODO: place app contents draw functions here.
			m_songNameRenderer->Draw();
			m_beatmapQueueNamesRenderer->Draw();


			// Show debug information to HUD (normaly not used).
			if (m_debugInfoVisible) {
				m_timeRenderer->Draw();
				m_fpsRenderer->Draw();
			}

			HRESULT hr = m_deviceResources->GetD2DRenderTarget()->EndDraw();
			if (hr == D2DERR_RECREATE_TARGET) {
				// If the render target failed to draw for any reason and a new target must be created.
				m_deviceResources->HandleDeviceLost();
			}
			else {
				ThrowIfFailed(hr);
			}

			m_deviceResources->m_drawing = FALSE;
		}
		catch (...) {
			// Oops threads are fighting against each other. sorry.
			// TODO: throw error if needed.

			// The threading need some more safety measures to prevent this.

			m_deviceResources->GetD2DRenderTarget()->Flush();
			m_deviceResources->m_drawing = FALSE;
		}
	}
	// Frame is ready return.
	return TRUE;
}


// Register the app window class.
WORD AppMain::MyRegisterClass(HINSTANCE hInstance, WNDPROC windowProc) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEXW);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_OSUBOT_V3));
	wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_SMALL));
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszClassName = m_windowClass;
	wcex.lpszMenuName = NULL;
	wcex.cbClsExtra = NULL;
	wcex.cbWndExtra = NULL;

	return RegisterClassExW(&wcex);
}

// Initialize and setup the app window.
bool AppMain::InitInstance(HINSTANCE hInstance, int nCmdShow) {
	// Store the instance as the app member variable.
	m_hInstance = hInstance;

	// Create the WINAPI window.
	m_hWnd = CreateWindowExW(
		WS_SYSMENU | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
		m_windowClass,
		m_title,
		WS_POPUP,					// Dissable for debugging with window.
		//WS_OVERLAPPEDWINDOW,		// Enable for debugging with window.
		m_rect.left, m_rect.top,
		(INT)roundf(m_windowSize.Width), (INT)roundf(m_windowSize.Height),
		NULL,
		NULL,
		m_hInstance,
		nullptr
	);

	if (!m_hWnd) {
		// Return when failed to create the window.
		return FALSE;
	}

	// Set the window transparency.
	SetLayeredWindowAttributes(m_hWnd, m_windowTransparencyColor, m_windowTransparencyAlpha, ULW_COLORKEY | LWA_ALPHA);

	// Keep window as top with topmost flag.
	// and resize window to desired size.
	SetWindowPos(m_hWnd, HWND_TOPMOST, m_rect.left, m_rect.top, (INT)roundf(m_windowSize.Width), (INT)roundf(m_windowSize.Height), SWP_ASYNCWINDOWPOS);

	// Show the window.
	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	// Return when succeeded to create and setup the app window.
	return TRUE;
}


// Get the rect for the working space of the app.
bool AppMain::GetWorkingRect() {
	RECT backupRect = m_rect;

	// Store working rect to app member rect.
	m_rect = { 0, 0, 800, 600 };

	if (m_osuBot) {
		if (m_osuBot->m_targetHwnd != NULL) {
			GetWindowRect(m_osuBot->m_targetHwnd, &m_rect);
		}
	}

	// Calculate and store the app width, height.
	m_windowSize.Width = (FLOAT)(m_rect.right - m_rect.left);
	m_windowSize.Height = (FLOAT)(m_rect.bottom - m_rect.top);


	// Only update sizes if content is initialized.
	if (m_osuBot && !EqualRect(&backupRect, &m_rect)) {
		// Rescale the app window to the new size.
		SetWindowPos(
			m_hWnd,
			HWND_TOPMOST,
			m_rect.left,
			m_rect.top,
			(INT)roundf(m_windowSize.Width),
			(INT)roundf(m_windowSize.Height),
			SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS
		);

		// Update window size dependent device resources.
		m_deviceResources->SetMainWindow();
		CreateWindowSizeDependentResources();
	}

	return TRUE;
}


// Notifies renderers that device resources need to be released.
void AppMain::OnDeviceLost() {
	// TODO: place contents device resources release funtions.
	m_songNameRenderer->ReleaseDeviceDependentResources();

	m_timeRenderer->ReleaseDeviceDependentResources();
	m_fpsRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void AppMain::OnDeviceRestored() {
	// TODO: place contents device resources creation functions.
	m_songNameRenderer->CreateDeviceDependentResources(m_windowTransparencyAlpha, D2D1::ColorF::Red, D2D1::ColorF::DarkGray);

	m_timeRenderer->CreateDeviceDependentResources(m_windowTransparencyAlpha, D2D1::ColorF::YellowGreen, D2D1::ColorF::Blue);
	m_fpsRenderer->CreateDeviceDependentResources(m_windowTransparencyAlpha, D2D1::ColorF::YellowGreen, D2D1::ColorF::Blue);

	// Recreate the window size dependent resources.
	CreateWindowSizeDependentResources();
}