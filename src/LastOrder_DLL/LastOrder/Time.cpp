//////////////////////////////////////////////////////////////////////////////
// Timer.h
// =======
// High Resolution Timer.
// This timer is able to measure the elapsed time with 1 micro-second accuracy
// in both Windows, Linux and Unix system 
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2003-01-13
// UPDATED: 2006-01-13
//
// Copyright (c) 2003 Song Ho Ahn
//////////////////////////////////////////////////////////////////////////////

#pragma once

#define _WINSOCKAPI_
#define NOMINMAX
#include <windows.h>
class Timer
{
	double startTimeInMicroSec;                 // starting time in micro-second
	double endTimeInMicroSec;                   // ending time in micro-second
	int    stopped;                             // stop flag 

	LARGE_INTEGER frequency;                    // ticks per second
	LARGE_INTEGER startCount;                   //
	LARGE_INTEGER endCount;                     //


public:

	Timer()
	{

		QueryPerformanceFrequency(&frequency);
		startCount.QuadPart = 0;
		endCount.QuadPart = 0;

		stopped = 0;
		startTimeInMicroSec = 0;
		endTimeInMicroSec = 0;

		//start();
	}

	~Timer() {}                                 // default destructor

	void start()
	{
		stopped = 0; // reset stop flag

		QueryPerformanceCounter(&startCount);

	}

	void stop()
	{
		stopped = 1; // set timer stopped flag


		QueryPerformanceCounter(&endCount);

	}

	double getElapsedTimeInMicroSec()
	{

		startTimeInMicroSec = startCount.QuadPart * (1000000.0 / frequency.QuadPart);
		endTimeInMicroSec = endCount.QuadPart * (1000000.0 / frequency.QuadPart);


		return endTimeInMicroSec - startTimeInMicroSec;
	}

	double getNoStopElapsedTimeInMicroSec()
	{

		if (!stopped)
		{
		QueryPerformanceCounter(&endCount);
		}


		startTimeInMicroSec = startCount.QuadPart * (1000000.0 / frequency.QuadPart);
		endTimeInMicroSec = endCount.QuadPart * (1000000.0 / frequency.QuadPart);


		return endTimeInMicroSec - startTimeInMicroSec;
	}

	double getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.001;
	}


	double getElapsedTimeInSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.000001;
	}

	double getElapsedTime()
	{
		return this->getElapsedTimeInSec();
	}
};




