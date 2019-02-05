// StaticText.h : Declares a UI Element that is just a static text.

#pragma once

#include <Common/Pch.h>

#include <Common/DeviceResources.h>


namespace OsuBot
{
	namespace UIElements
	{
		class StaticText {
		public:
			StaticText(
				_In_ const std::shared_ptr<DX::DeviceResources>& deviceResources,
				_In_opt_ BYTE alpha = 0xff,
				_In_opt_ D2D1::ColorF::Enum color = D2D1::ColorF::White,
				_In_opt_ DX::Size<FLOAT> maxSize = { 120.f, 10.f },
				_In_opt_ float fontSize = 32.f,
				_In_opt_ std::wstring text = L"",
				_In_opt_ bool background = FALSE,
				_In_opt_ RECT backgroundRect = { 0L, 0L, 120L, 10L },
				_In_opt_ D2D1::ColorF::Enum backgroundColor = D2D1::ColorF::Black,
				_In_opt_ DWRITE_TEXT_ALIGNMENT textAlignment = DWRITE_TEXT_ALIGNMENT_TRAILING,
				_In_opt_ std::wstring fontName = L"Source Code Pro",
				_In_opt_ std::wstring localeName = L"en_US"
				);

			void CreateDeviceDependentResources(_In_opt_ BYTE alpha = 0xff, _In_opt_ D2D1::ColorF::Enum color = D2D1::ColorF::White, _In_opt_ D2D1::ColorF::Enum backgroundColor = D2D1::ColorF::Black);
			void ReleaseDeviceDependentResources();
			void SetTranslation(DX::Size<FLOAT> translation);
			void Update(std::wstring newText = NULL);
			void Draw();

		private:
			// Cached pointer to device resources.
			std::shared_ptr<DX::DeviceResources> m_deviceResources;

			// Resources related to text rendering.
			bool												m_background;
			float												m_fontSize;
			std::wstring										m_text;
			std::wstring										m_fontName;
			std::wstring										m_localeName;
			DWRITE_TEXT_METRICS									m_textMetrics;
			DX::Size<FLOAT>										m_maxSize;
			RECT												m_backgroundRect;
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>		m_textBrush;
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>		m_backgroundBrush;
			Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1>		m_stateBlock;
			Microsoft::WRL::ComPtr<IDWriteTextLayout3>			m_textLayout;
			Microsoft::WRL::ComPtr<IDWriteTextFormat3>			m_textFormat;
			DWRITE_TEXT_ALIGNMENT								m_textAlignment;
			D2D1::Matrix3x2F									m_screenTranslation;
		};
	}
}