// DeviceResources.h : Declares everything that is needed
// for DirectX device resources.

#pragma once


namespace DX
{
	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi) {
		static const float dipsPerInch = 96.0f;
		return roundf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}
	

	// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
	__interface IDeviceNotify {
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};
	

	// Controls all DirectX device resources.
	class DeviceResources {
	public:
		DeviceResources();
		void SetMainWindow(_In_opt_ LPVOID main = nullptr);
		void HandleDeviceLost();
		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);

		// The size of the render target, in pixels.
		DX::Size<UINT>				GetOutputSize() const					{ return m_outputSize; }

		// The size of the render target, in dips.
		DX::Size<FLOAT>				GetLogicalSize() const					{ return m_logicalSize; }
		float						GetDpi() const							{ return m_effectiveDpi; }

		// D2D Accessors.
		ID2D1Factory3*				GetD2DFactory() const					{ return m_d2dFactory.Get(); }
		ID2D1HwndRenderTarget*		GetD2DRenderTarget() const				{ return m_d2dRenderTarget.Get(); }
		IDWriteFactory2*			GetDWriteFactory() const				{ return m_dwriteFactory.Get(); }
		D2D1::Matrix3x2F			GetOrientationTransform2D() const		{ return m_orientationTransform2D; }


	private:
		// Functions.
		void CreateDeviceIndependentResources();
		void CreateWindowSizeDependentResources();
		void UpdateRenderTargetSize();
		DXGI_MODE_ROTATION ComputeDisplayRotation();

		// Direct2D drawing components.
		Microsoft::WRL::ComPtr<ID2D1Factory3>			m_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget>	m_d2dRenderTarget;
		Microsoft::WRL::ComPtr<IDWriteFactory3>			m_dwriteFactory;

		// Cached device properties.
		DX::Size<UINT>									m_d2dRenderTargetSize;
		DX::Size<UINT>									m_outputSize;
		DX::Size<FLOAT>									m_logicalSize;
		float											m_dpi;
		float											m_effectiveDpi;

		// Transforms used for display.
		D2D1::Matrix3x2F								m_orientationTransform2D;

		// The IDeviceNotify can be held directly as it owns the DeviceResources.
		IDeviceNotify* m_deviceNotify;

	public:
		// Flag that indicates drawing started.
		bool m_drawing;
		bool m_resizeing;

		// Display orientation;
		DWORD m_currentOrientation;

		// Pointer to main app.
		LPVOID m_main;
	};
}