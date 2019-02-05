// App.cpp : Defines the entry point for the application
// and holds the message processor function.

#include <Common/Pch.h>

#include <Content/AppMain.h>


// Forward declarations.
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

// Global variables.
std::shared_ptr<OsuBot::AppMain> g_main;
std::shared_ptr<DX::DeviceResources> g_deviceResources;
bool g_tryOnce = TRUE;


// Main entry point of the app.
int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow
) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	// The return value is ignored, because we want to continue running in the
	// unlikely event that HeapSetInformation fails.
	//HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);


	// TODO: Place pre-init startup code here.

	// Intitalize and store class instances.
	g_deviceResources = std::make_shared<DX::DeviceResources>();
	g_main = std::make_shared<OsuBot::AppMain>(g_deviceResources);

	// Initialize the app strings.
	LoadStringW(hInstance, IDS_TITLE, g_main->m_title, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_OSUBOT_V3, g_main->m_windowClass, MAX_LOADSTRING);

	// Register the app class.
	g_main->MyRegisterClass(hInstance, WindowProc);
	
	// Initialize the app instance.
	if (g_main->GetWorkingRect()) {
		if (g_main->InitInstance(hInstance, nCmdShow)) {
			// Initialize the app contents.
			g_main->InitContent();
		}
		else {
			// Cleanup and throw error code.


			// Stop the application.
			return FALSE;
		}
	}

	// Register the app hotkeys.
	RegisterHotKey(g_main->m_hWnd, 1, MOD_NOREPEAT, VK_HOME);					// Toggle HUD.
	RegisterHotKey(g_main->m_hWnd, 2, MOD_NOREPEAT, VK_TAB);					// Toggle fps counter.
	RegisterHotKey(g_main->m_hWnd, 3, MOD_NOREPEAT, VK_INSERT);					// Open file dialog to queue a beatmap.


	// Initialize threads.
	std::thread bot([&]() {
		while (!g_main->m_quit) {
			g_main->m_osuBot->CheckGameActive(&g_main);

			if (g_main->m_osuBot->m_targetHwnd) {
				g_main->m_osuBot->AutoPlay(); 
			}
		}
	});
	std::thread app([&]() {
		while (!g_main->m_quit) {
			g_main->Update();
			g_main->Draw();
		}
	});


	// Main message loop.
	MSG msg = { 0 };
	while (true) {
		// Look for messages in the queue.
		if (PeekMessageW(&msg, g_main->m_hWnd, 0U, 0U, PM_REMOVE)) {
			// Handle the message.
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		// Stop loop when flagged.
		else if (g_main->m_quit) {
			bot.join();
			app.join();
			break;
		}
	}

	g_deviceResources.reset();
	g_main.reset();

	// Return with last code.
	return (INT)msg.wParam;
}

// Main app window message processor.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_DISPLAYCHANGE:
	{
		// Update the working rect.
		g_main->GetWorkingRect();
	}
	case WM_HOTKEY:
	{
		// Toggle HUD.
		if (LOWORD(lParam) == NULL && HIWORD(lParam) == VK_HOME) {
			if (g_main->m_hudVisible) {
				// Dissable HUD intractability.
				SetWindowLongW(g_main->m_hWnd, GWL_EXSTYLE, WS_SYSMENU | WS_EX_TRANSPARENT);

				// Set flag to not draw the HUD.
				g_main->m_hudVisible = FALSE;


				// Clear the HUD from the frame.
				g_deviceResources->GetD2DRenderTarget()->BeginDraw();
				g_deviceResources->GetD2DRenderTarget()->Clear(D2D1::ColorF(D2D1::ColorF::Black));
				HRESULT hr = g_deviceResources->GetD2DRenderTarget()->EndDraw();

				if (hr == D2DERR_RECREATE_TARGET) {
					// If the render target failed to draw for any reason and a new target must be created.
					g_deviceResources->HandleDeviceLost();
				}
				else {
					ThrowIfFailed(hr);
				}
			}
			else {
				// Enable HUD intractability.
				SetWindowLongW(g_main->m_hWnd, GWL_EXSTYLE, WS_SYSMENU);

				// Set flag to draw the HUD.
				g_main->m_hudVisible = TRUE;
			}

			// Set the window with the new parameters.
			SetWindowPos(
				g_main->m_hWnd,
				HWND_TOPMOST,
				g_main->m_rect.left, g_main->m_rect.top,
				(INT)roundf(g_main->m_windowSize.Width), (INT)roundf(g_main->m_windowSize.Height),
				SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS
			);
		}

		// Toggle fps counter.
		if (LOWORD(lParam) == NULL && HIWORD(lParam) == VK_TAB) {
			if (g_main->m_debugInfoVisible) {
				// Set flag to not show fps counter.
				g_main->m_debugInfoVisible = FALSE;
			}
			else {
				// Set flag to show fps counter.
				g_main->m_debugInfoVisible = TRUE;
			}
		}

		// Open file dialog to queue a beatmap.
		if (LOWORD(lParam) == NULL && HIWORD(lParam) == VK_INSERT) {
			// Get the path to a beatmap file.
			std::wstring path = g_main->m_osuBot->GetSongFromFolderPath();
			g_main->m_timer.ResetElapsedTime();
			g_main->m_osuBot->m_logicTimer.ResetElapsedTime();

			g_main->m_osuBot->AddBeatmapToQueue(path);
		}
		break;
	}
	case WM_DESTROY:
	{
		// Post quit message to message queue.
		PostQuitMessage(0);

		// Set flag to quit the app.
		g_main->m_quit = TRUE;
		break;
	}
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	return FALSE;
}