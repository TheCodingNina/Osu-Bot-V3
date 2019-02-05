// Beatmap.h : Declares the classes that holds the usefull information
// from a beatmap for the Bot.

#pragma once

#include <Common/Vec2f.h>
#include <Common/SplitString.h>

#include <fstream>


namespace OsuBot
{
	namespace BeatmapInfo
	{
		// A class that holds the points of a slider segment.
		class Segment {
		public:
			// Constructor.
			Segment(std::vector<vec2f> points) : m_points(points) {};

			// Member variables.
			std::vector<vec2f> m_points;
		};
		
		// A class that holds information about a timing point for a hit object.
		class TimingPoint {
		public:
			// Constructor.
			explicit TimingPoint(_In_ std::wstring timingString);

			// Accessor functions.
			int		GetTime() const		{ return m_time; }
			float	GetBpm() const		{ return m_bpm; }
			

		private:
			// Member variables.
			int m_time;
			float m_bpm;
		};

		// A class that holds information about a hit object from a beatmap.
		class HitObject {
		public:
			// Constructor.
			HitObject(
				_In_ std::wstring hitString,
				_In_ std::vector<TimingPoint>* timingPoints,
				_In_ float beatmapSliderMultiplier,
				_In_ float beatmapSliderTickRate
			);

		private:
			// Internal functions.
			// Slider info functions.
			void GetSliderInfo(
				_In_ std::vector<std::wstring>* tokens,
				_In_ std::vector<TimingPoint>* timingPoints,
				_In_ float beatmapSliderMultiplier,
				_In_ float beatmapSliderTickRate
			);
			void GetLinearSliderInfo(_In_ std::vector<vec2f>* sliderPoints);
			void GetCircularSliderInfo(_In_ std::vector<vec2f>* sliderPoints);
			void GetBezierSliderInfo(_In_ std::vector<vec2f>* sliderPoints);

			// Spinner info function.
			void GetSpinnerInfo(_In_ std::vector<std::wstring>* tokens);


		public:
			// Get point on slider by time.
			vec2f GetPointByT(_In_ const double& time) const;

			// Accessor functions.
			int			GetObjectType() const;

			vec2f		GetStartPosition() const			{ return m_startPosition; }
			vec2f		GetEndPosition() const;

			int			GetStartTime() const				{ return m_startTime; }
			int			GetEndTime() const;
			int			GetSliderTime() const				{ return m_sliderTime; }
			float		GetSliderTickCount() const			{ return m_sliderTickCount; }
			UINT		GetSliderRepeatCount() const		{ return m_sliderRepeatCount;}
			UINT		GetStackIndex() const				{ return m_stackIndex; }


		private:
			// Member variables.
			vec2f m_sliderCenter;
			vec2f m_startPosition;
			int m_startTime;
			int m_endTime;
			int m_sliderTime;
			float m_startAngle;
			float m_endAngle;
			float m_sliderRadius;
			float m_pixelLenght;
			float m_beatLenght;
			float m_beatLenghtBase;
			float m_sliderTickCount;
			UINT m_sliderRepeatCount;
			UINT m_stackIndex;
			UINT m_objectType;
			BYTE m_sliderType;
			std::vector<Segment> m_sliderSegments;
		};
		
		// A class that holds all usefull information about a beatmap for the bot.
		class Beatmap {
		public:
			// Constructor and destructor.
			Beatmap(const wchar_t* path) :
				m_filePath(path)
			{
				m_beatmapFile = _wfsopen(m_filePath, L"r", _SH_DENYNO);
			}
			~Beatmap() {
				if (m_beatmapFile) {
					fclose(m_beatmapFile);
				}
			}

			// Member functions.
			bool ParseBeatmap();


		public:
			// Accessor functions.
			const HitObject* FindHitObjectAtT(_In_ const double& songTime) const;
			const HitObject* GetHitObjectAtIndex(_In_ const UINT& index) const;
			
			UINT GetHitObjectsCount() const { return (UINT)m_hitObjects.size(); }

			float GetStackOffset() const { return m_stackOffset; }
			float GetCircleSize() const { return m_circleSize; }

			std::wstring		GetTitle() const				{ return m_title; }
			std::wstring		GetArtist() const				{ return m_artist; }
			std::wstring		GetCreator() const				{ return m_creator; }
			std::wstring		GetVersion() const				{ return m_version; }
			UINT				GetBeatmapID() const			{ return m_beatmapID; }
		
		private:
			// Internal function.
			bool FindHeader(_In_ const UINT& headerIndex);
			void ReadKeysUnderHeader(_In_ UINT& headerIndex);
			std::wstring GetValueString(_In_ const std::wstring* readLine) const;


		private:
			// Header enum and wstring array.
			const enum beatmapHeaders : UINT {
				General,
				Editor,
				Metadata,
				Difficulty,
				TimingPoints,
				Colours,
				HitObjects,
				count
			};

			constexpr static const wchar_t* headerStrings[beatmapHeaders::count] = {
				L"[General]",
				L"[Editor]",
				L"[Metadata]",
				L"[Difficulty]",
				L"[TimingPoints]",
				L"[Colours]",
				L"[HitObjects]"
			};

		public:
			// Member variables.
			FILE* m_beatmapFile;
			LPCWSTR m_filePath;
			float m_stackOffset;

		private:
			// Beatmap headers.
			bool m_headerGeneral;
			bool m_headerEditor;
			bool m_headerMetadata;
			bool m_headerDifficulty;
			bool m_headerTimingPoints;
			bool m_headerColours;
			bool m_headerHitObjects;

			// General header.
			float m_stackLeniency;
			UINT m_gameMode;

			// Editor header.
			float m_beatDivisor;

			// Metadata header.
			std::wstring m_title;
			std::wstring m_artist;
			std::wstring m_creator;
			std::wstring m_version;
			UINT m_beatmapID;

			// Difficulty header.
			float m_circleSize;
			float m_overallDifficulty;
			float m_approachRate;
			float m_sliderMultiplier;
			float m_sliderTickRate;

			// Timingpoints header.
			std::vector<TimingPoint> m_timingPoints;

			// HitObjects header.
			std::vector<HitObject> m_hitObjects;
		};


		// Usefull functions with vec2f objects.

		// calculates the binomial coefficient.
		inline double BinomialCoefficient(const UINT& n, const UINT& i) {
			if (i < 0U || i > n) {
				return 0.0;
			}
			else if (i == 0U || i == n) {
				return 1.0;
			}

			double c = 1.0;
			for (UINT u = 0U; u < min(i, n - i); u++) {
				c = c * static_cast<double>(n - u) / static_cast<double>(u + 1U);
			}

			return c;
		}

		// Calculates the Bernstein using a binomial coefficient.
		inline double Bernstein(const UINT& i, const UINT& n, const double& t) {
			return BinomialCoefficient(n, i) * pow(t, static_cast<double>(i)) * pow(1.0 - t, static_cast<double>(n) - static_cast<double>(i));
		}

		// Returns the point on a bezier curve
		// with time 0 - 1.
		inline vec2f GetPointOnBezier(
			_In_ const std::vector<vec2f>& bezier,
			_In_ const double& time,
			_In_opt_ const UINT& repeatCount = 0U
		) {
			vec2f point(0.f, 0.f);
			UINT pointsSize = (UINT)bezier.size() - 1U;

			// Calculate the point using a berstein calculation.
			for (UINT i = 0U; i <= pointsSize; i++) {
				// Translate the point for each segment.
				double b = Bernstein(i, pointsSize, time);
				point.Add(bezier.at(i + (repeatCount * pointsSize)).Copy().Mult(static_cast<FLOAT>(b)));
			}

			return point;
		}

		// Returns the point on a circle
		// with angle in radians.
		inline vec2f GetPointOnCircle(
			_In_ const vec2f& center,
			_In_ const float& radius,
			_In_ const float& angle
		) {
			vec2f point;
			// Calculate the point on a circle.
			// Using unit circle calculus.
			point.X = radius * cosf(angle);
			point.Y = radius * sinf(angle);

			// Translate the point with the center vec2f.
			point.Add(center);

			return point;
		}

		// Returns the intersecting point bewteen the vec2f objects.
		inline vec2f Intersect(
			_In_ const vec2f& vec1,
			_In_ const vec2f& vec2,
			_In_ const vec2f& vec3,
			_In_ const vec2f& vec4
		) {
			float d = vec4.X * vec2.Y - vec4.Y * vec2.X;
			if (fabsf(d) < 0.00001f) {
				// D is too small.
				// This means that the vec2f objects can be called parallel.
				// TODO: Thow error if needed.

				// The function will continue, but might give an unexpected result.
			}
			
			float u = ((vec3.Y - vec1.Y) * vec2.X + (vec1.X - vec3.X) * vec2.Y) / d;
			
			return vec3.Copy().Add(vec4.Copy().Mult(u));
		}

		// Check if the mid angle is inside the start and end angles.
		inline bool IsInside(
			_In_ const float& startAngle,
			_In_ const float& midAngle,
			_In_ const float& endAngle
		) {
			return (midAngle > startAngle && midAngle < endAngle) || (midAngle < startAngle && midAngle > endAngle);
		}

		// Check if both segments (points at same index) are exactly equal.
		inline bool operator== (const Segment& lhs, const Segment& rhs) {
			if (lhs.m_points.size() != rhs.m_points.size()) {
				// The segments have different sizes.
				return FALSE;
			}
			for (UINT i = 0U; i < lhs.m_points.size(); i++) {
				if (lhs.m_points.at(i) != rhs.m_points.at(i)) {
					// The points in the segment are not equal.
					return FALSE;
				}
			}
			
			// All points (at same index) in the segments are equal.
			return TRUE;
		}

		// Check if both hitObjects are the exact same.
		inline bool operator== (const HitObject& lhs, const HitObject& rhs) {
			// If any important data mismatches, return FALSE.
			if (lhs.GetEndTime() != rhs.GetEndTime()) {
				return FALSE;
			}
			if (lhs.GetEndPosition() != rhs.GetEndPosition()) {
				return FALSE;
			}
			if (lhs.GetObjectType() != rhs.GetObjectType()) {
				return FALSE;
			}
			if (lhs.GetStartPosition() != rhs.GetStartPosition()) {
				return FALSE;
			}
			if (lhs.GetStartTime() != rhs.GetStartTime()) {
				return FALSE;
			}

			// If no data mismatched, return TRUE.
			return TRUE;
		}
	}
}