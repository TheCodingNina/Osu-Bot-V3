// DeviceResources.cpp : Holds the definitions for all the
// DirectX device resources.

#include <Common/Pch.h>

#include <Common/DeviceResources.h>
#include <Content/AppMain.h>

using namespace DX;


namespace DisplayMetrics
{
	// High resolution displays can require a lot of GPU and battery power to render.
	// High resolution phones, for example, may suffer from poor battery life if
	// games attempt to render at 60 frames per second at full fidelity.
	// The decision to render at full fidelity across all platforms and form factors
	// should be deliberate.
	static const bool SupportHighResolutions = false;

	// The default thresholds that define a "high resolution" display. If the thresholds
	// are exceeded and SupportHighResolutions is false, the dimensions will be scaled
	// by 50%.
	static const float DpiThreshold = 192.0f;		// 200% of standard desktop display.
	static const float WidthThreshold = 1920.0f;	// 1080p width.
	static const float HeightThreshold = 1080.0f;	// 1080p height.
};

// Constructor for DeviceResources.
DeviceResources::DeviceResources() :
	m_logicalSize(),
	m_outputSize(),
	m_dpi(96.f),
	m_effectiveDpi(-1.f),
	m_deviceNotify(nullptr)
{
	CreateDeviceIndependentResources();
}

// Configures independent resources.
void DeviceResources::CreateDeviceIndependentResources() {
	// Initiallize Direct2D resources.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the projects is in a debug build, enable Direct2D debugging via SKD layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	// Initialize the Direct2D Factory.
	ThrowIfFailed(
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory3),
			&options,
			&m_d2dFactory
		)
	);

	// Initialize the DirectWrite Factory.
	ThrowIfFailed(
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory3),
			&m_dwriteFactory
		)
	);
}

// These resources need to be recreated every time we resize the window.
void DeviceResources::CreateWindowSizeDependentResources() {
	UpdateRenderTargetSize();

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d2dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d2dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;


	// Create the render target properties.
	D2D1_RENDER_TARGET_PROPERTIES rtp;
	ZeroMemory(&rtp, sizeof(rtp));
	rtp.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
	rtp.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	rtp.dpiX = m_effectiveDpi;
	rtp.dpiY = m_effectiveDpi;
	rtp.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
	rtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

	// Create the hwnd render target properties.
	D2D1_HWND_RENDER_TARGET_PROPERTIES hrtp;
	ZeroMemory(&hrtp, sizeof(hrtp));
	hrtp.hwnd = reinterpret_cast<OsuBot::AppMain*>(m_main)->m_hWnd;
	hrtp.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY;
	hrtp.pixelSize = D2D1::SizeU(
		(UINT32)roundf(reinterpret_cast<OsuBot::AppMain*>(m_main)->m_windowSize.Width),
		(UINT32)roundf(reinterpret_cast<OsuBot::AppMain*>(m_main)->m_windowSize.Height)
	);

	// Create a new render target.
	HRESULT hr = GetD2DFactory()->CreateHwndRenderTarget(
		&rtp,
		&hrtp,
		&m_d2dRenderTarget
	);
	
	if (hr == D2DERR_RECREATE_TARGET) {
		// If the render target failed to create for any reason, a new target must be created.
		HandleDeviceLost();

		// Everything is setup now. Do NOT continue execution of this function.
		// HandleDeviceLost has re-entered the function and correctly setup the new device.
		return;
	}
	else {
		ThrowIfFailed(hr);
	}


	// Set the proper orientation for the transforms.
	switch (displayRotation) {
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform2D = D2D1::Matrix3x2F::Identity();
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform2D = D2D1::Matrix3x2F::Rotation(90.f) * D2D1::Matrix3x2F::Translation(m_logicalSize.Height, 0.f);
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform2D = D2D1::Matrix3x2F::Rotation(180.f) * D2D1::Matrix3x2F::Translation(m_logicalSize.Width, m_logicalSize.Height);
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform2D = D2D1::Matrix3x2F::Rotation(270.f) * D2D1::Matrix3x2F::Translation(0.f, m_logicalSize.Width);
		break;

	default:
		m_orientationTransform2D = D2D1::Matrix3x2F::Identity();
		break;
	}
}

// Determine the dimensions of the render target and whether it will be scaled down.
void DeviceResources::UpdateRenderTargetSize() {
	m_effectiveDpi = m_dpi;

	// To improve battery life on high resolution devices, render to a smaller render target
	// and allow the GPU to scale the output when it is presented.
	if (!DisplayMetrics::SupportHighResolutions && m_dpi > DisplayMetrics::DpiThreshold) {
		float width = ConvertDipsToPixels((FLOAT)m_logicalSize.Width, m_dpi);
		float height = ConvertDipsToPixels((FLOAT)m_logicalSize.Height, m_dpi);

		// When the device is in portrait orientation, height > width. Compare the
		// larger dimension against the width threshold and the smaller dimension
		// against the height threshold.
		if (max(width, height) > DisplayMetrics::WidthThreshold && min(width, height) > DisplayMetrics::HeightThreshold) {
			// To scale the app we change the effective DPI. Logical size does not change.
			m_effectiveDpi /= 2.0f;
		}
	}

	// Calculate the necessary render target size in pixels.
	m_outputSize.Width = (UINT)roundf(DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi));
	m_outputSize.Height = (UINT)roundf(DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi));

	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1U);
	m_outputSize.Height = max(m_outputSize.Height, 1U);
}

// This function should be called when the main window is recreated (or resized).
void DeviceResources::SetMainWindow(_In_opt_ LPVOID main) {
	if (main != nullptr) {
		m_main = main;
	}

	m_logicalSize = reinterpret_cast<OsuBot::AppMain*>(m_main)->m_windowSize;
	m_dpi = (FLOAT)GetDpiForWindow(reinterpret_cast<OsuBot::AppMain*>(m_main)->m_hWnd);

	CreateWindowSizeDependentResources();
}

// Recreate all device resources and set them back to the current state.
void DeviceResources::HandleDeviceLost() {
	if (m_deviceNotify != nullptr) {
		m_deviceNotify->OnDeviceLost();
	}

	CreateWindowSizeDependentResources();

	if (m_deviceNotify != nullptr) {
		m_deviceNotify->OnDeviceRestored();
	}
}

// Register our DeviceNotify to be informed on device lost and creation.
void DeviceResources::RegisterDeviceNotify(IDeviceNotify* deviceNotify) {
	m_deviceNotify = deviceNotify;
}

// This function determines the rotation between the display device's native orientation and the
// current display orientation.
DXGI_MODE_ROTATION DX::DeviceResources::ComputeDisplayRotation() {
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// Note: NativeOrientation can only be Landscape or Portrait even though
	// the DisplayOrientations enum has other values.
	switch (m_currentOrientation) {
		case DMDO_DEFAULT:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DMDO_90:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DMDO_180:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DMDO_270:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;
		}
	return rotation;
}