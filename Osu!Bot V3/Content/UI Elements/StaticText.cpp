// StaticText.cpp : Defines everything about the StaticText class.

#include <Common/Pch.h>

#include <Content/UI Elements/StaticText.h>


using namespace OsuBot::UIElements;


StaticText::StaticText(
	_In_ const std::shared_ptr<DX::DeviceResources>& deviceResources,
	_In_opt_ BYTE alpha,
	_In_opt_ D2D1::ColorF::Enum color,
	_In_opt_ DX::Size<FLOAT> maxSize,
	_In_opt_ float fontSize,
	_In_opt_ std::wstring text,
	_In_opt_ bool background,
	_In_opt_ RECT backgroundRect,
	_In_opt_ D2D1::ColorF::Enum backgroundColor,
	_In_opt_ DWRITE_TEXT_ALIGNMENT textAlignment,
	_In_opt_ std::wstring fontName,
	_In_opt_ std::wstring localeName
) :
	m_deviceResources(deviceResources),
	m_maxSize(maxSize),
	m_screenTranslation(D2D1::Matrix3x2F::Identity()),
	m_text(text),
	m_background(background),
	m_backgroundRect(backgroundRect),
	m_textAlignment(textAlignment)
{
	ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

	// Create device independent resources.
	Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
	ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			fontName.c_str(),
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fontSize,
			localeName.c_str(),
			&textFormat
		)
	);

	ThrowIfFailed(
		textFormat.As(&m_textFormat)
	);

	ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
	);

	ThrowIfFailed(
		m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
	);

	CreateDeviceDependentResources(alpha, color, backgroundColor);
}


// Change the translation of the text.
void StaticText::SetTranslation(DX::Size<FLOAT> translation) {
	m_screenTranslation = D2D1::Matrix3x2F::Translation(
		translation.Width,
		translation.Height
	);
}

// Update the static text with new parameters.
void StaticText::Update(
	std::wstring newText
) {
	// Update displayed text.
	if (!newText.empty()) {
		m_text = newText;

		Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
		ThrowIfFailed(
			m_deviceResources->GetDWriteFactory()->CreateTextLayout(
				m_text.c_str(),
				(uint32_t)m_text.length(),
				m_textFormat.Get(),
				m_maxSize.Width,
				m_maxSize.Height,
				&textLayout
			)
		);

		ThrowIfFailed(
			textLayout.As(&m_textLayout)
		);

		ThrowIfFailed(
			m_textLayout->GetMetrics(&m_textMetrics)
		);
	}
}

// Draw the text to the display.
void StaticText::Draw() {
	auto renderTarget = m_deviceResources->GetD2DRenderTarget();
	if (renderTarget == nullptr) return;

	renderTarget->SaveDrawingState(m_stateBlock.Get());

	renderTarget->SetTransform(m_screenTranslation * m_deviceResources->GetOrientationTransform2D());

	ThrowIfFailed(
		m_textFormat->SetTextAlignment(m_textAlignment)
	);

	if (m_textLayout.Get() && m_textBrush.Get()) {
		if (m_background) {
			renderTarget->FillRectangle(
				D2D1::RectF(
					(FLOAT)m_backgroundRect.left,
					(FLOAT)m_backgroundRect.top,
					(FLOAT)m_backgroundRect.right,
					(FLOAT)m_backgroundRect.bottom
				),
				m_backgroundBrush.Get()
			);
		}

		renderTarget->DrawTextLayout(
			D2D1::Point2F(0.f, 0.f),
			m_textLayout.Get(),
			m_textBrush.Get()
		);
	}

	renderTarget->RestoreDrawingState(m_stateBlock.Get());
}


// Create the device dependent resources for the text.
void StaticText::CreateDeviceDependentResources(BYTE alpha, D2D1::ColorF::Enum color, D2D1::ColorF::Enum backgroundColor) {
	ThrowIfFailed(
		m_deviceResources->GetD2DRenderTarget()->CreateSolidColorBrush(D2D1::ColorF(color, (FLOAT)alpha / 0xff), &m_textBrush)
	);
	
	ThrowIfFailed(
		m_deviceResources->GetD2DRenderTarget()->CreateSolidColorBrush(D2D1::ColorF(backgroundColor, (FLOAT)alpha / 0xff), &m_backgroundBrush)
	);
}

// Release the device dependent resoures for the text.
void StaticText::ReleaseDeviceDependentResources() {
	m_textBrush.Reset();
}