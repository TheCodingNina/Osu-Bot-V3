// MovementModes.h : Declares a class that holds everything that
// involves the movement of the cursor for OsuBot.

#pragma once

#include <Common/Vec2f.h>


namespace OsuBot
{
	// Forward declare the bot class.
	class Bot;

	class MovementModes {
	public:
		// Base movement functions.
		void MoveToObject(Bot* bot, vec2f (MovementModes::*callback)(const UINT&));
		void MovementSlider(Bot* bot, vec2f(MovementModes::*callback)(const UINT&));
		void MovementSpinner(Bot* bot, vec2f(MovementModes::*callback)(const UINT&));

		// TODO: Add movement variant calculation functions here.
		vec2f ControlPointStandard(const UINT& index);
		vec2f ControlPointFlowing(const UINT& index);
		vec2f ControlPointPredicting(const UINT& index);

		// Hermite interpolation function.
		double HermiteInterpolation(const double& time, const double& bias = 0.3);

	public:
		// Member variables.
		BYTE m_movementModeCircle;
		BYTE m_movementModeSlider;
		BYTE m_movementModeSpinner;
		float m_spinnerRadius;
	private:
		bool m_interpolateTime;
		double m_savedSongTime;
		double m_startTime;
		float m_currentRadius;
		float m_currentAngle;

		// Frequently used vec2f objects for calculations.
		vec2f m_beginPoint;
		vec2f m_endPoint;
		vec2f m_controlPoint0;
		vec2f m_controlPoint1;
		vec2f m_previousPoint;
		vec2f m_nextPoint;
		vec2f m_spinnerCenter;
	public:
		vec2f m_backupPoint;

	public:
		// Bezier pts vector.
		std::vector<vec2f> m_bezierPts;
		std::vector<vec2f> m_sliderPoints;
	};
}