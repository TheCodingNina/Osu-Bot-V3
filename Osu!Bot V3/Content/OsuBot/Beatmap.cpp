// Beatmap.cpp : Defines the content in Beatmap.h

#include <Common/Pch.h>

#include <Content/OsuBot/Beatmap.h>


using namespace OsuBot::BeatmapInfo;


// Timing point constructor.
TimingPoint::TimingPoint(_In_ std::wstring timingString) {
	// Split the timingString into tokens.
	std::vector<std::wstring> tokens = SplitString(timingString, L",");

	// Get the time from the tokens at index [0].
	m_time = std::stoi(tokens.at(0));

	// Try to get the bpm value from the tokens at index [1].
	try {
		m_bpm = std::stof(tokens.at(1));
	}
	catch (...) {
		// Could not convert from string to float with std::stof().
		// This means that the value is either: 
		//		Greater than FLT_MAX.
		//		Less than 0.
		//		Almost one of the two.

		if (tokens.at(1).find('-') != std::wstring::npos) {
			// The string contains a negative value, set the bpm to 0.
			m_bpm = 0.f;
		}
		else {
			// Set the bpm to the maximum float value.
			m_bpm = FLT_MAX;
		}
	}
}



// Hit object constructor.
HitObject::HitObject(
	_In_ std::wstring hitString,
	_In_ std::vector<TimingPoint>* timingPoints,
	_In_ float beatmapSliderMultiplier,
	_In_ float beatmapSliderTickRate
) :
	m_sliderCenter(0.f, 0.f),
	m_startPosition(0.f, 0.f),
	m_startTime(0),
	m_endTime(0),
	m_sliderTime(0),
	m_startAngle(0.f),
	m_endAngle(0.f),
	m_sliderRadius(0.f),
	m_pixelLenght(0.f),
	m_beatLenght(0.f),
	m_beatLenghtBase(0.f),
	m_sliderTickCount(0.f),
	m_sliderRepeatCount(0U),
	m_stackIndex(0U),
	m_objectType(0U),
	m_sliderType(0x00)
{
	// Split the hitString into tokens.
	std::vector<std::wstring> tokens = SplitString(hitString, L",");

	// Get the start positions and time values of the object.
	m_startPosition = vec2f(std::stof(tokens.at(0)), std::stof(tokens.at(1)));
	m_startTime = std::stoi(tokens.at(2));

	// Get the object type.
	m_objectType = _wtoi(tokens.at(3).c_str());

	if (GetObjectType() == HITOBJECT_SLIDER) {
		// The object is a slider, get the required information from the hitString.
		// and Calculate other required information.
		GetSliderInfo(&tokens, timingPoints, beatmapSliderMultiplier, beatmapSliderTickRate);
	}
	else if (GetObjectType() == HITOBJECT_SPINNER) {
		// The object is a spinner, get the end time from the hitString.
		GetSpinnerInfo(&tokens);
	}
	else {
		// Oops something went wrong with the object type.
		// TODO: Thow error if needed.
	}
}

// This function should only be called when the object is a slider.
// The function gets all the required information and stores it in the HitObject class.
void HitObject::GetSliderInfo(
	_In_ std::vector<std::wstring>* tokens,
	_In_ std::vector<TimingPoint>* timingPoints,
	_In_ float beatmapSliderMultiplier,
	_In_ float beatmapSliderTickRate
) {
	// Get the beat lenght base.
	float bpm = m_beatLenghtBase = timingPoints->at(0).GetBpm();

	// Get the repeat count.
	m_sliderRepeatCount = (UINT)std::stoi(tokens->at(6));

	// Get the pixel lenght.
	m_pixelLenght = std::stof(tokens->at(7));

	for (TimingPoint point : *timingPoints) {
		// Check if the timing point is valid.
		if (point.GetTime() <= m_startTime) {
			if (point.GetBpm() >= 0.f) {
				// Valid timimg point, set the beat lenght base to the bpm.
				m_beatLenghtBase = point.GetBpm();
			}
			// Always set bpm to the timing points bpm.
			bpm = point.GetBpm();
		}
	}

	if (bpm < 0.f) {
		// Calculate the correct bpm.
		bpm = m_beatLenghtBase * bpm / -100.f;
	}
	// Set the beat lenght.
	m_beatLenght = bpm;


	// Calculate the slider end time and slider time (duration).
	m_sliderTime = (INT)roundf(m_beatLenght * (m_pixelLenght / beatmapSliderMultiplier) / 100.f);
	m_endTime = (INT)roundf(static_cast<FLOAT>(m_sliderTime) * m_sliderRepeatCount) + m_startTime;

	// Calculate the slider tick count and limit the minimum value to 1.0f.
	m_sliderTickCount = m_pixelLenght / (((100.f * beatmapSliderMultiplier) / beatmapSliderTickRate) / (m_beatLenght / m_beatLenghtBase));
	m_sliderTickCount = max(m_sliderTickCount, 1.f);


	// Push the start position to the slider points vector.
	std::vector<vec2f> sliderPoints;
	sliderPoints.push_back(m_startPosition);

	// Split the tokens at index [5] into tokens that hold the slider points.
	std::vector<std::wstring> sliderTokens = SplitString(tokens->at(5), L"|");
	// Start at index 1 to skip the slider type byte.
	for (UINT i = 1U; i < (UINT)sliderTokens.size(); i++) {
		// Split the point string into tokens that hold the point x and y values.
		std::vector<std::wstring> pointTokens = SplitString(sliderTokens.at(i), L":");

		// Create a vec2f point from the point tokens.
		vec2f point(std::stof(pointTokens.at(0)), std::stof(pointTokens.at(1)));

		// Push the point the slider points vector.
		sliderPoints.push_back(point);
	}

	// Remove the last point back, if it is the same as the second to last one.
	if (sliderPoints.at(sliderPoints.size() - 1U) == sliderPoints.at(sliderPoints.size() - 2U)) {
		sliderPoints.pop_back();
	}


	// Get the slider type from the slider tokens.
	m_sliderType = (BYTE)sliderTokens.at(0).front();

	// Do the calculations for the correct slider type.
	if (m_sliderType == 0x4C || m_sliderType == 0x43) {
		// Slider is of type L'L' (0x4C) or L'C' (0x43).
		// This means the slider has only linear segments.
		GetLinearSliderInfo(&sliderPoints);
	}
	else if (m_sliderType == 0x50 && sliderPoints.size() == 3U) {
		// Slider is of type L'P' (0x50) and has exactly 3 points.
		// This means the slider has a circluar body.
		GetCircularSliderInfo(&sliderPoints);
	}
	else {
		m_sliderType = 0x42;
		// Slider type does not require specific calculations.
		// And is set to L'B' (0x42).
		// This means the slider body can be calculated using bezier curves.
		GetBezierSliderInfo(&sliderPoints);
	}
}

// This function should only be called when the slider has only linear segements.
// The function gets the segment points and stores it in the m_sliderSegemnts vector.
void HitObject::GetLinearSliderInfo(_In_ std::vector<vec2f>* sliderPoints) {
	for (UINT i = 1; i < (UINT)sliderPoints->size(); i++) {
		// Create a segment with the slider points.
		Segment seg({ sliderPoints->at(i - 1U), sliderPoints->at(i) });

		// Push the segment to the slider segments vector.
		m_sliderSegments.push_back(seg);
	}
}

// This function should only be called when the slider has only circular segments.
// The function calculates:
//		Slider center				stores into m_sliderCenter.
//		Slider starting angle		stores into m_startAngle.
//		Slider ending angle			stores into m_endAngle.
//		Slider radius				stores into m_sliderRadius.
void HitObject::GetCircularSliderInfo(_In_ std::vector<vec2f>* sliderPoints) {
	// Calculate slider center.
	vec2f midA = sliderPoints->at(0).MidPoint(sliderPoints->at(1));
	vec2f midB = sliderPoints->at(2).MidPoint(sliderPoints->at(1));
	vec2f norA = sliderPoints->at(1).Copy().Sub(sliderPoints->at(0)).Nor();
	vec2f norB = sliderPoints->at(1).Copy().Sub(sliderPoints->at(2)).Nor();

	m_sliderCenter = Intersect(midA, norA, midB, norB);

	// Calculate the slider angles.
	vec2f startAnglePoint = sliderPoints->at(0).Copy().Sub(m_sliderCenter);
	vec2f midAnglePoint = sliderPoints->at(1).Copy().Sub(m_sliderCenter);
	vec2f endAnglePoint = sliderPoints->at(2).Copy().Sub(m_sliderCenter);

	m_startAngle = atan2f(startAnglePoint.Y, startAnglePoint.X);
	float midAngle = atan2f(midAnglePoint.Y, midAnglePoint.X);
	m_endAngle = atan2f(endAnglePoint.Y, endAnglePoint.X);

	// Correct the angles.
	if (!IsInside(m_startAngle, midAngle, m_endAngle)) {
		if (fabsf(m_startAngle + M_2PI - m_endAngle) < M_2PI && IsInside(m_startAngle + M_2PI, midAngle, m_endAngle)) {
			m_startAngle += M_2PI;
		}
		else if (fabsf(m_startAngle - (m_endAngle + M_2PI)) < M_2PI && IsInside(m_startAngle, midAngle, m_endAngle + M_2PI)) {
			m_endAngle += M_2PI;
		}
		else if (fabsf(m_startAngle - M_2PI - m_endAngle) < M_2PI && IsInside(m_startAngle - M_2PI, midAngle, m_endAngle)) {
			m_startAngle -= M_2PI;
		}
		else if (fabsf(m_startAngle - (m_endAngle - M_2PI)) < M_2PI && IsInside(m_startAngle, midAngle, m_endAngle - M_2PI)) {
			m_endAngle -= M_2PI;
		}
		else {
			// Something when wrong with correcting the angles.
			// TODO: Thow error if needed.

			// The function will continue, but might give an unexpected result.
		}
	}
	// Last correction with the arcing angle.
	if (m_endAngle > m_startAngle) {
		m_endAngle = m_startAngle + (m_pixelLenght / startAnglePoint.Length());
	}
	else {
		m_endAngle = m_startAngle - (m_pixelLenght / startAnglePoint.Length());
	}

	// Get the slider radius.
	m_sliderRadius = startAnglePoint.Length();
}

// This function should be called when the slider has neither only linear or circular segments.
// The function calculates the curves and stores the segments in the m_sliderSegments vector.
void HitObject::GetBezierSliderInfo(_In_ std::vector<vec2f>* sliderPoints) {
	std::vector<std::vector<vec2f>> curveList;
	std::vector<vec2f> curve;

	for (vec2f point : *sliderPoints) {
		// Store the points in the curve.
		if ((UINT)curve.size() > 1U) {
			if (point == curve.at(curve.size() - 1U)) {
				// Point is the last point in the curve.
				// Store the curve in the curveList.
				curveList.push_back(curve);
				curve.clear();
			}
		}
		curve.push_back(point);
	}
	// Store the remaining part of the curve in the curveList.
	curveList.push_back(curve);
	curve.clear();

	// Store the curves from the curveList in the slider segments.
	for (Segment seg : curveList) {
		m_sliderSegments.push_back(seg);
	}
}


// This function should only be called when the object is a spinner.
// The function gets the end time of the spinner and stores it in the HitObject class.
void HitObject::GetSpinnerInfo(_In_ std::vector<std::wstring>* tokens) {
	// Get the spinner end time.
	m_endTime = _wtoi(tokens->at(5).c_str());
}


// This function is used to get the point on a slider at a specified time.
vec2f HitObject::GetPointByT(_In_ const double& time) const {
	double pointTime = time; //static_cast<int>(floor(time)) % 2 == 0 ? time - floor(time) : floor(time) + 1.0 - time;

	if (m_sliderType == 0x50) {
		// Slider has a circluar body.
		// Construct a point using unit circle calculus.
		float angle = m_startAngle * static_cast<float>(1.0 - pointTime) + m_endAngle * static_cast<float>(pointTime);
		return vec2f(m_sliderCenter.X + m_sliderRadius * cosf(angle), m_sliderCenter.Y + m_sliderRadius * sinf(angle));
	}

	// For other slider types do:

	// Check if m_sliderSegments is valid.
	vec2f oldPoint;
	try {
		oldPoint = m_sliderSegments.at(0).m_points.at(0);
	}
	catch (...) {
		// Something went wrong with accessing the first point in the slider segment vector.
		// TODO: Thow error if needed.
		OutputDebugStringW(L"ERROR: oldPoint (sliderSegments) failed!\n");

		// Return from the function with a pre-determined point.
		return GetStartPosition();
	}

	double currentDistance = 0.0;
	double pointPixelLength = (DOUBLE)m_pixelLenght * pointTime;

	for (auto seg : m_sliderSegments) {
		if (seg == m_sliderSegments.back()) {
			double currentTime = 0.0;
			while (currentDistance < m_pixelLenght) {
				vec2f p = GetPointOnBezier(seg.m_points, currentTime);
				currentDistance += (oldPoint - p).Length();

				if (currentDistance > pointPixelLength) {
					return oldPoint;
				}

				oldPoint = p;
				currentTime += 1.0 / static_cast<double>(seg.m_points.size() * 50U - 1U);
			}
		}

		for (double currentTime = 0.0; currentTime < 1.0 + (1.0 / static_cast<double>(seg.m_points.size() * 50U - 1U)); currentTime += (1.0 / static_cast<double>(seg.m_points.size() * 50U - 1U))) {
			vec2f p = GetPointOnBezier(seg.m_points, currentTime);

			currentDistance += (oldPoint - p).Length();
			if (currentDistance > pointPixelLength) {
				return oldPoint;
			}

			oldPoint = p;
		}
	}

	return oldPoint;
}


// This function is used to get the hit object type.
// The result is either:
//		HITOBJECT_CIRCLE	with value 1
//		HITOBJECT_SLIDER	with value 2
//		HITOBJECT_SPINNER	with value 8
int HitObject::GetObjectType() const {
	if ((m_objectType & 2) > 0) {
		// The object is a slider.
		return HITOBJECT_SLIDER;
	}
	else if ((m_objectType & 8) > 0) {
		// The object is a spinner.
		return HITOBJECT_SPINNER;
	}
	else {
		// The object is a circle.
		return HITOBJECT_CIRCLE;
	}
}

// This function is used to get the end position of the object.
// If the object type is NOT a slider, return the start position instead.
vec2f HitObject::GetEndPosition() const {
	if (GetObjectType() == HITOBJECT_SLIDER) {
		// Object is a slider, return the slider end position.
		return GetPointByT(static_cast<const float>(GetSliderRepeatCount()));
	}
	// Object is not a slider, return the start position of the object.
	return GetStartPosition();
}

// This function is used to get the end time of the object.
// If the object type is a circle, return the start time instead.
// NOTE: circles don't have an end time specified.
int HitObject::GetEndTime() const {
	if (GetObjectType() != HITOBJECT_CIRCLE) {
		// Object is a slider, return the slider end time.
		return m_endTime;
	}
	// Object is not a slider, return the start time of the object.
	return m_startTime;
}


// This function should be called to parse the beatmap information
// then optionaly pushing the beatmap to a queue.
bool Beatmap::ParseBeatmap() {
	// Create a index for the headers.
	UINT headerIndex = 0U;

	if (m_beatmapFile != nullptr) {
		// Open success, start reading.
		while (TRUE) {
			if (feof(m_beatmapFile)) {
				// End of file.
				// TODO: Send a notifier if needed.

				// Return early from the function.
				break;
			}
			else {
				// Find the headers.
				if (FindHeader(headerIndex)) {
					// Read the keys under the header.
					ReadKeysUnderHeader(headerIndex);

					// Set index for the next header.
					headerIndex++;

					// Reset all header flags.
					m_headerGeneral = FALSE;
					m_headerEditor = FALSE;
					m_headerMetadata = FALSE;
					m_headerDifficulty = FALSE;
					m_headerTimingPoints = FALSE;
					m_headerColours = FALSE;
					m_headerHitObjects = FALSE;
				}
			}
		}
	}
	else {
		// Couldn't open file, reset the FILE.
		return FALSE;
	}

	// After parsing, there needs to be some calculations done for the stacking of the HitObjects.
	// TODO: Make a correctly working one this time :stuck_out_tongue_winking_eye:

	// (The calculations in V2 didn't seem to be correctly working.)
	// This shouldn't have a too large impact on the autoplaying.


	// Calculate the stacking offset with the circle size.
	m_stackOffset = ((512.0f / 16.0f) * (1.0f - 0.7f * (m_circleSize - 5.0f) / 5.0f) / 10.0f) / m_circleSize;

	return TRUE;
}

// This function is used to find a header in a opened beatmap.
bool Beatmap::FindHeader(_In_ const UINT& headerIndex) {
	// Create a buffer to assign.
	std::wstring readLine;
	LPWSTR lpReadLine = new wchar_t[MAX_READSTRING];

	// Reset the cursor position.
	fpos_t fpos = 0U;
	fsetpos(m_beatmapFile, &fpos);


	// Find the header at the array with the index.
	while (readLine.find(headerStrings[headerIndex]) == std::wstring::npos) {
		if (feof(m_beatmapFile)) {
			// End of file, return false.

			delete lpReadLine;
			return FALSE;
		}
		else {
			// Read a new line.
			auto debugstr = fgetws(lpReadLine, MAX_READSTRING, m_beatmapFile);
			readLine.assign(lpReadLine);
		}
	}

	// Header found, flag the header.
	switch (headerIndex) {
	case beatmapHeaders::General:
		m_headerGeneral = TRUE;
		break;

	case beatmapHeaders::Editor:
		m_headerEditor = TRUE;
		break;

	case beatmapHeaders::Metadata:
		m_headerMetadata = TRUE;
		break;

	case beatmapHeaders::Difficulty:
		m_headerDifficulty = TRUE;
		break;

	case beatmapHeaders::TimingPoints:
		m_headerTimingPoints = TRUE;
		break;

	case beatmapHeaders::Colours:
		m_headerColours = TRUE;
		break;

	case beatmapHeaders::HitObjects:
		m_headerHitObjects = TRUE;
		break;
	}

	delete lpReadLine;
	return TRUE;
}

// This function reads the values under the specified header from the beatmap.
void Beatmap::ReadKeysUnderHeader(_In_ UINT& headerIndex) {
	// Create a buffer to assign.
	LPWSTR lpReadLine = new wchar_t[MAX_READSTRING];
	std::wstring readLine;

	// Calculate next header index (don't if next index out of range).
	UINT nextIndex = headerIndex;
	if (headerIndex + 1U != beatmapHeaders::count) {
		nextIndex = headerIndex + 1U;
	}

	// Read until the next header.
	while (readLine.find(headerStrings[nextIndex]) == std::wstring::npos) {
		if (feof(m_beatmapFile)) {
			// End of file, return.
			return;
		}
		else {
			// Read a new line.
			auto debugstr = fgetws(lpReadLine, MAX_READSTRING, m_beatmapFile);
			readLine.assign(lpReadLine);
		}

		// Read the keys under the current header, if the header was found.
		switch (headerIndex) {
		case beatmapHeaders::General:
			if (m_headerGeneral) {
				if (readLine.find(L"StackLeniency") != std::wstring::npos) {
					m_stackLeniency = std::stof(GetValueString(&readLine));
				}
				else if (readLine.find(L"Mode") != std::wstring::npos) {
					m_gameMode = std::stoi(GetValueString(&readLine));
				}
			}
			break;

		case beatmapHeaders::Editor:
			if (m_headerEditor) {
				if (readLine.find(L"BeatDivisor") != std::wstring::npos) {
					m_beatDivisor = std::stof(GetValueString(&readLine));
				}
			}
			break;

		case beatmapHeaders::Metadata:
			if (m_headerMetadata) {
				if (readLine.find(L"Title") != std::wstring::npos) {
					if (readLine.find(L"Unicode") == std::wstring::npos) {
						m_title = GetValueString(&readLine);
					}
				}
				else if (readLine.find(L"Artist") != std::wstring::npos) {
					if (readLine.find(L"Unicode") == std::wstring::npos) {
						m_artist = GetValueString(&readLine);
					}
				}
				else if (readLine.find(L"Creator") != std::wstring::npos) {
					m_creator = GetValueString(&readLine);
				}
				else if (readLine.find(L"Version") != std::wstring::npos) {
					m_version = GetValueString(&readLine);
				}
				else if (readLine.find(L"BeatmapID") != std::wstring::npos) {
					m_beatmapID = (UINT)std::stoi(GetValueString(&readLine));
				}
			}
			break;

		case beatmapHeaders::Difficulty:
			if (m_headerDifficulty) {
				if (readLine.find(L"CircleSize") != std::wstring::npos) {
					m_circleSize = std::stof(GetValueString(&readLine));
				}
				else if (readLine.find(L"OverallDifficulty") != std::wstring::npos) {
					m_overallDifficulty = std::stof(GetValueString(&readLine));
				}
				else if (readLine.find(L"ApproachRate") != std::wstring::npos) {
					m_approachRate = std::stof(GetValueString(&readLine));
				}
				else if (readLine.find(L"SliderMultiplier") != std::wstring::npos) {
					m_sliderMultiplier = std::stof(GetValueString(&readLine));
				}
				else if (readLine.find(L"SliderTickRate") != std::wstring::npos) {
					m_sliderTickRate = std::stof(GetValueString(&readLine));
				}
			}
			break;

		case beatmapHeaders::TimingPoints:
			if (m_headerTimingPoints) {
				if (readLine.find(L',') != std::wstring::npos) {
					m_timingPoints.push_back(TimingPoint(readLine));
				}
			}
			break;

		case beatmapHeaders::HitObjects:
			if (m_headerHitObjects) {
				if (readLine.find(L',') != std::wstring::npos) {
					m_hitObjects.push_back(HitObject(readLine, &m_timingPoints, m_sliderMultiplier, m_sliderTickRate));
				}
			}
			break;
		}
	}
	delete lpReadLine;
}

// Returns a string of the value from a key-value string (the immediate string from a beatmap).
std::wstring Beatmap::GetValueString(_In_ const std::wstring* readLine) const {
	auto str = readLine->substr(readLine->find(L":") + 1U);

	// Pop back the endline character.
	if (str.back() == L'\n') {
		str.pop_back();
	}

	return str;
}


// This function should be used to get a pointer to the hit object at specified time.
const HitObject* Beatmap::FindHitObjectAtT(_In_ const double& songTime) const {
	for (auto object = m_hitObjects.begin(); object != m_hitObjects.end(); ++object) {
		if (object->GetStartTime() >= songTime && object->GetObjectType() == HITOBJECT_CIRCLE) {
			// The object is a circle and song time is smaller than the start time.
			return &(*object);
		}
		else if (object->GetEndTime() > songTime) {
			// Continue if the object end time is greater than the song time.
			continue;
		}

		// Check if the object is not a circle and the end time smaller than the song time.
		if (object->GetEndTime() <= songTime && object->GetObjectType() != HITOBJECT_CIRCLE) {
			// Object found, return a pointer to it.
			return &(*object);
		}
		else if (*object == m_hitObjects.back()) {
			// The object is the last in the list (most likely already passed).
			return &(*object);
		}
	}

	// No object found, return a nullptr.
	return nullptr;
}

// This function retrives a pointer to a hitObject at a given index.
const HitObject* Beatmap::GetHitObjectAtIndex(_In_ const UINT& index) const {
	if (m_hitObjects.size() > 0U) {
		if (index < 0U) {
			// Index is smaller than 0U, return the first object.
			return &m_hitObjects.front();
		}
		else if (index >= m_hitObjects.size()) {
			// Index is greater than the size, return the last object.
			return &m_hitObjects.back();
		}

		// Return the object at index.
		return &m_hitObjects.at(index);
	}
	// No hitObjects in the vector, return a nullptr.
	return nullptr;
}