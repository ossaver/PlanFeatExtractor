#ifndef UTILS_H
#define UTILS_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* April 2022                                           */
/********************************************************/
/* Constants and utilities.								*/
/********************************************************/

#include <string.h>
#include <time.h>
#include <cstdint>
#include <limits>
#include <math.h>

const float 		EPSILON = 0.001f;
const unsigned int 	MAX_UNSIGNED_INT = std::numeric_limits<unsigned int>::max();
const int32_t 		MAX_INT32 = std::numeric_limits<int32_t>::max();
const float			FLOAT_INFINITY = std::numeric_limits<float>::infinity();
const float			FLOAT_UNKNOWN = std::numeric_limits<float>::lowest();
const uint16_t		MAX_UINT16 = 65535;

using TMutex = uint64_t;
using TOrdering = uint32_t;
using TVarValue = uint32_t;
using TPlanId = uint32_t;
using TTimePoint = uint16_t;
using TStep = uint16_t;
using TVariable = uint16_t;
using TValue = uint16_t;
using TTime = float;
using TFloatValue = float;

#define toSeconds(t) (float) (((int) (1000 * (clock() - t)/(float) CLOCKS_PER_SEC))/1000.0)

inline TTimePoint stepToStartPoint(TStep step) {	// Step number -> start time point
	return step << 1;
}

inline TTimePoint stepToEndPoint(TStep step) {		// Step number -> end time point
	return (step << 1) + 1;
}

inline TStep timePointToStep(TTimePoint t) {
	return t >> 1;
}

inline TTimePoint firstPoint(TOrdering ordering) {
	return ordering & 0xFFFF;
}

inline TTimePoint secondPoint(TOrdering ordering) {
	return ordering >> 16;
}

inline TOrdering getOrdering(TTimePoint p1, TTimePoint p2) {
	return (((TOrdering)p2) << 16) + p1;
}

class Utils {
public:
	static bool compareIgnoreCase(char* s1, const char* s2)
	{
		int l = (int)strlen(s1);
		if (l != strlen(s2)) return false;
		for (int i = 0; i < l; i++)
			if (tolower(s1[i]) != tolower(s2[i])) return false;
		return true;
	}

	static bool compareIgnoreCase(std::string& s1, const char* s2)
	{
		int l = (int)s1.length();
		if (l != (int)strlen(s2)) return false;
		for (int i = 0; i < l; i++)
			if (tolower(s1[i]) != tolower(s2[i])) return false;
		return true;
	}

	static bool compareIgnoreCase(std::string& str1, std::string& str2) {
		return ((str1.size() == str2.size()) && equal(str1.begin(), str1.end(), str2.begin(), [](char& c1, char& c2) {
			return (c1 == c2 || tolower(c1) == tolower(c2));
			}));
	}

	static int percentage(int value, float min, float max) {
		int maxValue = (int)round(value * max);
		int minValue = (int)round(value * min);
		return rand() % (maxValue - minValue + 1) + minValue;
	}
};

#endif
