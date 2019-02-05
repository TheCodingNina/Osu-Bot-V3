// MovementModes.h : Defines the movement functions of the cursor for OsuBot.

#include <Common/Pch.h>

#include <Content/OsuBot/MovementModes.h>

#include <Content/OsuBot.h>


using namespace OsuBot;



// Movement function to move to the next object.
void MovementModes::MoveToObject(Bot* bot, vec2f(MovementModes::*callback)(const UINT&)) {
	// Calculate the bezier pts if needed.
	if (m_bezierPts.size() == 0U) {
		// Retrive local pointers to
		//		object before last (object that came before the last one),
		//		previous (object that just ended),
		//		current (object to move to),
		//		next (object that comes after this move)
		// hitobjects.
		const BeatmapInfo::Beatmap* beatmap = bot->GetBeatmapAtIndex(bot->m_selectedBeatmapIndex);
		const BeatmapInfo::HitObject* objectBeforeLast = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex - 2U);
		const BeatmapInfo::HitObject* previousObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex - 1U);
		const BeatmapInfo::HitObject* currentObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex);
		const BeatmapInfo::HitObject* nextObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex + 1U);


		// Calculte the previous, begin, end and next points.
		if (bot->m_hitObjectIndex == 0U) {
			// Get the current cursor position.
			GetCursorPos(&bot->m_cursorPosition);

			m_previousPoint = m_beginPoint = m_backupPoint = vec2f(static_cast<FLOAT>(bot->m_cursorPosition.x), static_cast<FLOAT>(bot->m_cursorPosition.y));
		}
		else {
			// Calculate the previous point.
			if (previousObject->GetObjectType() == HITOBJECT_SLIDER) {
				double previousPointTime = ((DOUBLE)previousObject->GetSliderTickCount() - 1.0) / (DOUBLE)previousObject->GetSliderTickCount();
				previousPointTime = previousObject->GetSliderRepeatCount() % 2 == 0 ? 1.0 - previousPointTime : previousPointTime;
				m_previousPoint = previousObject->GetPointByT(previousPointTime);
				m_previousPoint.ConvertToWindowSpace(beatmap->GetStackOffset(), previousObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());
			}
			else if (bot->m_hitObjectIndex != 1U) {
				// The previous point is the object before the last object.
				m_previousPoint = objectBeforeLast->GetEndPosition();
				m_previousPoint.ConvertToWindowSpace(beatmap->GetStackOffset(), objectBeforeLast->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());
			}

			// The end point of the previous move (current cursor position) becomes the begin point of this move.
			GetCursorPos(&bot->m_cursorPosition);
			m_beginPoint = vec2f(static_cast<FLOAT>(bot->m_cursorPosition.x), static_cast<FLOAT>(bot->m_cursorPosition.y));

			//m_beginPoint = previousObject->GetEndPosition();
			//m_beginPoint.ConvertToWindowSpace(beatmap->GetStackOffset(), previousObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());
		}

		m_endPoint = currentObject->GetStartPosition();
		m_endPoint.ConvertToWindowSpace(beatmap->GetStackOffset(), currentObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());

		if (currentObject->GetObjectType() == HITOBJECT_SLIDER) {
			double nextPointTime = 1.0 / max(1.0, (FLOAT)currentObject->GetSliderTickCount());
			m_nextPoint = currentObject->GetPointByT(nextPointTime);
			m_nextPoint.ConvertToWindowSpace(beatmap->GetStackOffset(), currentObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());
		}
		else {
			m_nextPoint = nextObject->GetStartPosition();
			m_nextPoint.ConvertToWindowSpace(beatmap->GetStackOffset(), nextObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());
		}


		m_interpolateTime = TRUE;

		// Calculate the control point(s).
		m_controlPoint0 = m_beginPoint.Copy().Sub(m_backupPoint).Add(m_beginPoint);

		if (m_endPoint.Copy().Sub(m_beginPoint).Length() < (1.f / beatmap->GetCircleSize() * 400.f)) {
			m_controlPoint1 = ControlPointFlowing(1U);
			m_controlPoint0 = m_beginPoint.Copy().Sub(m_backupPoint).Normalize().Mult(m_endPoint.Copy().Sub(m_beginPoint).Length() / 2.f).Add(m_beginPoint);

			m_interpolateTime = FALSE;
		}
		else {
			m_controlPoint1 = (bot->*callback)(1U);
		}

		// Overwrite controlPoints for a linear move.
		if (m_movementModeCircle == MODE_STANDARD) {
			m_controlPoint0 = (bot->*callback)(0U);
			m_controlPoint1 = (bot->*callback)(1U);
		}

		// Fill the bezier pts vector.
		m_bezierPts = {
			m_beginPoint,
			m_controlPoint0,
			m_controlPoint1,
			m_endPoint
		};

		// Save controlPoint1 into backupPoint for next object.
		m_backupPoint = m_controlPoint1;

		// Save the current song time to calculate the time delta.
		m_savedSongTime = bot->GetSongTime();
		m_startTime = currentObject->GetStartTime();
	}

	// Calculate the time (0.0 - 1.0) until current object should be hit.
	double deltaTime = m_startTime - m_savedSongTime;
	double time = (deltaTime - (m_startTime - bot->GetSongTime())) / deltaTime;

	if (m_interpolateTime) {
		// Interpolate the time with a hermite curve.
		time = HermiteInterpolation(time);
	}

	// Clamp the time between 0.0 - 1.0.
	time = CLAMP(0.0, time, 1.0);


	// Get the next point on the bezier curve.
	vec2f bezierPoint = BeatmapInfo::GetPointOnBezier(m_bezierPts, time);
	vec2f resultPoint = bezierPoint;
	vec2f newPoint = bezierPoint;

	vec2f sliderPointCurrent, sliderPointPrevious;

	// Blend the resultPoint with the sliderPoint.
	const BeatmapInfo::Beatmap* beatmap = bot->GetBeatmapAtIndex(bot->m_selectedBeatmapIndex);
	const BeatmapInfo::HitObject* previousObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex - 1U);
	const BeatmapInfo::HitObject* currentObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex);
	if (currentObject->GetObjectType() == HITOBJECT_SLIDER && m_interpolateTime) {
		// Movement into slider.
		sliderPointCurrent = currentObject->GetPointByT(time - 1.0);
		sliderPointCurrent.ConvertToWindowSpace(beatmap->GetStackOffset(), currentObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());

		// Store sliderPoint in newPoint.
		newPoint = sliderPointCurrent;

		// Blend the points into a result.
		time = HermiteInterpolation(time, -0.6);
		resultPoint = bezierPoint.Copy().Mult(static_cast<float>(1.0 - time)).Add(sliderPointCurrent.Copy().Mult(static_cast<float>(time)));
	}
	if (previousObject->GetObjectType() == HITOBJECT_SLIDER && m_interpolateTime) {
		// Movement out of slider.
		sliderPointPrevious = previousObject->GetPointByT(time + 1.0);
		sliderPointPrevious.ConvertToWindowSpace(beatmap->GetStackOffset(), previousObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());

		// Blend the points into a result.
		// NOTICE: Use newPoint instead of bezierPoint, so that movement out of slider can blend with movement into slider.
		time = HermiteInterpolation(time, 0.6);
		resultPoint = sliderPointPrevious.Copy().Mult(static_cast<float>(1.0 - time)).Add(newPoint.Copy().Mult(static_cast<float>(time)));
	}
	
	// Set the cursor to the result point.
	SetCursorPos(static_cast<int>(resultPoint.X), static_cast<int>(resultPoint.Y));
}

// Movement function to move along a slider.
void MovementModes::MovementSlider(Bot* bot, vec2f(MovementModes::*callback)(const UINT&)) {
	// NOTICE: Movement modes not yet implemented!
	UNREFERENCED_PARAMETER(callback);
	
	// Execute different code for standard slider mode.
	if (m_movementModeSlider == MODE_STANDARD) {
		// Retrive local pointers to the current object (slider).
		const BeatmapInfo::Beatmap* beatmap = bot->GetBeatmapAtIndex(bot->m_selectedBeatmapIndex);
		const BeatmapInfo::HitObject* currentObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex);

		// Calculate the time (0.0 - 1.0), slider repeats are handled after.
		double time = (bot->GetSongTime() - currentObject->GetStartTime()) / currentObject->GetSliderTime();
		time = CLAMP(0.0, time, (DOUBLE)currentObject->GetSliderRepeatCount());
		time = static_cast<int>(floor(time)) % 2 == 0 ? time - floor(time) : floor(time) + 1.0 - time;
		time = CLAMP(0.0, time, 1.0);

		// Calculate the next point on the slider.
		vec2f resultPoint = currentObject->GetPointByT(time);
		resultPoint.ConvertToWindowSpace(beatmap->GetStackOffset(), currentObject->GetStackIndex(), bot->GetMultiplier(), bot->GetOffset());

		// Setthe cursor to the correct point on the slider body.
		SetCursorPos(static_cast<int>(resultPoint.X), static_cast<int>(resultPoint.Y));
	}
	else {
		// Calculate the slider point if needed.
		//if () {
		//	// Retrive local pointers to
		//	//		object before last (object that came before the last one),
		//	//		previous (object that just ended),
		//	//		current (object to move to),
		//	//		next (object that comes after this move)
		//	// hitobjects.
		//	const BeatmapInfo::Beatmap* beatmap = bot->GetBeatmapAtIndex(bot->m_selectedBeatmapIndex);
		//	const BeatmapInfo::HitObject* objectBeforeLast = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex - 2U);
		//	const BeatmapInfo::HitObject* previousObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex - 1U);
		//	const BeatmapInfo::HitObject* currentObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex);
		//	const BeatmapInfo::HitObject* nextObject = beatmap->GetHitObjectAtIndex(bot->m_hitObjectIndex + 1U);
		//}
	}
}

// Movement function to spin the spinners.
void MovementModes::MovementSpinner(Bot* bot, vec2f(MovementModes::*callback)(const UINT&)) {
	// NOTICE: Movement modes not yet implemented!
	UNREFERENCED_PARAMETER(callback);

	// Calculate the spinner center if needed.
	if (m_spinnerCenter == vec2f()) {
		const BeatmapInfo::HitObject* currentObject = bot->GetBeatmapAtIndex(bot->m_selectedBeatmapIndex)->GetHitObjectAtIndex(bot->m_hitObjectIndex);

		// Set the radius with the beatmap circle size.
		m_currentRadius = m_spinnerRadius * (1.f / bot->GetBeatmapAtIndex(bot->m_selectedBeatmapIndex)->GetCircleSize());

		m_spinnerCenter = currentObject->GetStartPosition();
		m_spinnerCenter.Add(0.f, 6.f); // Offset the center the back from the global offset (POINT w = { 0, 6 } @ CheckGameActive() in OsuBot.cpp).
		m_spinnerCenter.ConvertToWindowSpace(0.f, 0U, bot->GetMultiplier(), bot->GetOffset());
	}

	// Calculate the next rotation point in the spinner.
	vec2f resultPoint = vec2f(cosf(m_currentAngle) * m_currentRadius, sinf(m_currentAngle) * m_currentRadius);
	resultPoint.Add(m_spinnerCenter);

	// Spin the spinner.
	SetCursorPos(static_cast<int>(resultPoint.X), static_cast<int>(resultPoint.Y));

	// Modify the current angle of the spinner.
	m_currentAngle += M_PI / -12.f;
}


// Returns a control point that follows a linear movement.
vec2f MovementModes::ControlPointStandard(const UINT& index) {
	vec2f cp0 = m_beginPoint.Copy().Mult(0.667f).Add(m_endPoint.Copy().Mult(0.334f));
	vec2f cp1 = m_beginPoint.Copy().Mult(0.334f).Add(m_endPoint.Copy().Mult(0.667f));

	return index ? cp0 : cp1;
}

// Returns a control point that follows a flowing movement.
vec2f MovementModes::ControlPointFlowing(const UINT& index) {
	UNREFERENCED_PARAMETER(index);

	vec2f d0 = m_previousPoint.Copy().Sub(m_beginPoint);
	vec2f d1 = m_beginPoint.Copy().Sub(m_endPoint);
	vec2f d2 = m_endPoint.Copy().Sub(m_nextPoint);

	float l0 = d0.Length();
	float l1 = d1.Length();
	float l2 = d2.Length();

	vec2f m0 = m_previousPoint.MidPoint(m_beginPoint);
	vec2f m1 = m_beginPoint.MidPoint(m_endPoint);
	vec2f m2 = m_endPoint.MidPoint(m_nextPoint);

	float amplifier0 = (atan2f(l2 / 480.f, 1.85f * (l2 / 960.f)) / ((40000.f / 1.f) / l1)) + 1.f;
	float amplifier1 = (atan2f(l1 / 480.f, 1.85f * (l1 / 960.f)) / ((40000.f / 1.f) / l1)) + 1.f;

	vec2f cp0 = m1 + (m_beginPoint - (m1 + (m0 - m1) * ((l1 * amplifier1) / (l0 + l1))));
	vec2f cp1 = m0 + (m_beginPoint - (m1 + (m0 - m1) * ((l1 * amplifier1) / (l0 + l1))));
	vec2f cp2 = m2 + (m_endPoint - (m2 + (m1 - m2) * ((l2 * amplifier0) / (l1 + l2))));
	vec2f cp3 = m1 + (m_endPoint - (m2 + (m1 - m2) * ((l2 * amplifier0) / (l1 + l2))));

	// We only need cp3 for controlPoint1 for the bezier curve (for now).
	return cp3;
}

// Returns a control point that follows an movement that looks to be able to predict the next movement.
vec2f MovementModes::ControlPointPredicting(const UINT& index) {
	UNREFERENCED_PARAMETER(index);

	// Big complicated calculation that cannot be explaned.
	// As it was made with mostly trial and error (what looked good/bad).
	// And I also forgot why I did these steps :stuck_out_tongue_winking_eye:
	return m_nextPoint.MidPoint(m_endPoint).Sub(m_nextPoint).Mult(m_beginPoint.Copy().Sub(m_endPoint).Length() / (860.f / 1.f)).Add(m_endPoint).MidPoint(m_controlPoint0);
}


// Interpolates a double with a hermite curve.
double MovementModes::HermiteInterpolation(const double& _X, const double& bias) {
	// TODO: Change thse 6 variables to customize the hermite curve.
	double _Y0 = 0.1;	// In target
	double _Y1 = 0.0;	// Start
	double _Y2 = 1.0;	// End
	double _Y3 = 1.1;	// Out target

	double tension	=  -0.2;	// 1 : high, 0 : normal, -1 : low
	//double bias		=	0.3;	// >0 : first segment, 0 : mid, <0 : next segment


	double a0, a1, a2, a3;
	double m0, m1;

	double _X2 = _X * _X;
	double _X3 = _X2 * _X;

	m0	= (_Y1 - _Y0) * (1.0 + bias) * (1.0 - tension) / 2.0;
	m0 += (_Y2 - _Y1) * (1.0 - bias) * (1.0 - tension) / 2.0;
	m1	= (_Y2 - _Y1) * (1.0 + bias) * (1.0 - tension) / 2.0;
	m1 += (_Y3 - _Y2) * (1.0 - bias) * (1.0 - tension) / 2.0;

	a0 =  2.0 *	_X3 - 3.0 * _X2 + 1.0;
	a1 =		_X3 - 2.0 * _X2 + _X;
	a2 =		_X3 -		_X2;
	a3 = -2.0 * _X3 + 3.0 * _X2;

	// Calculate the resulting value on the hermite curve.
	double _Y = (a0 * _Y1 + a1 * m0 + a2 * m1 + a3 * _Y2);

	return CLAMP(0.0, _Y, 1.0);
}