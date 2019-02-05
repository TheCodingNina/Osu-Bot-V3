// SplitString.h : Defines a funtion that splits a std::string on a delimiter
// into a std::vector<std::string>. Also has std::wstring support.

#pragma once

#include <vector>


// SplitString for std::string.
inline std::vector<std::string> SplitString(
	_In_ const std::string& str,
	_In_opt_ const std::string& delimiter = " ",
	_In_opt_ size_t startPosition = 0
) {
	std::vector<std::string> result;

	if (startPosition >= str.size()) {
		// The startPosition is beyond the last character in the string, return empty result.
		return result;
	}

	size_t currentPosition = 0;
	while (str.find(delimiter, startPosition + 1U) != std::string::npos) {
		// Delimiter found, push the substring to the result vector.
		result.push_back(str.substr(currentPosition, str.find(delimiter, startPosition + 1U) - currentPosition));

		// Move the startPosition and currentPosition for the next segment.
		currentPosition = startPosition = str.find(delimiter, startPosition + 1U) + 1U;
	}

	// Push the last remaining segment to the result vector.
	result.push_back(str.substr(startPosition));

	// return the result with the splitted string.
	return result;
}

// SplitString for std::wstring. (WIDE CONVERSIONS).
inline std::vector<std::wstring> SplitString(
	_In_ const std::wstring& str,
	_In_opt_ const std::wstring& delimiter = L" ",
	_In_opt_ size_t startPosition = 0
) {
	std::vector<std::wstring> result;

	if (startPosition >= str.size()) {
		// The startPosition is beyond the last character in the string, return empty result.
		return result;
	}

	size_t currentPosition = 0;
	while (str.find(delimiter, startPosition + 1U) != std::wstring::npos) {
		// Delimiter found, push the substring to the result vector.
		result.push_back(str.substr(currentPosition, str.find(delimiter, startPosition + 1U) - currentPosition));

		// Move the startPosition and currentPosition for the next segment.
		currentPosition = startPosition = str.find(delimiter, startPosition + 1U) + 1U;
	}

	// Push the last remaining segment to the result vector.
	result.push_back(str.substr(startPosition));

	// return the result with the splitted string.
	return result;
}