#pragma once
#include <BWAPI.h>

#include <string>
#include <sstream>
#include <ctime>
#include <list>
#include <vector>
#include <map>

#include "Time.cpp"
#include "Common.h"

using namespace BWAPI;
using namespace std;


class TimerManager
{

	std::vector<Timer> timers;
	std::vector<std::string> timerNames;

	int barWidth;
	double maxTimeConsume;
	double itemMaxTimeConsume;
	int maxFrameCount;
	std::string maxTimerName;

public:

	enum Type { All, Worker, Production, Building, Attack, Scout, Information, tactic, strategy, Astar1, MutaliskTac, ZerglingTac, MutaliskRetreat, MutaAttack, NumTypes };

	TimerManager() : timers(std::vector<Timer>(NumTypes)), barWidth(40)
	{
		timerNames.push_back("Total");
		timerNames.push_back("Worker");
		timerNames.push_back("Production");
		timerNames.push_back("Building");
		timerNames.push_back("Attack");
		timerNames.push_back("Scout");
		timerNames.push_back("Information");
		timerNames.push_back("tactic");
		timerNames.push_back("strategy");
		timerNames.push_back("Astar1");
		timerNames.push_back("MutaliskTac");
		timerNames.push_back("ZerglingTac");
		timerNames.push_back("MutaliskRetreat");
		timerNames.push_back("MutaAttack");
		itemMaxTimeConsume = 0;
		maxTimeConsume = 0;
		maxTimerName = "";
		maxFrameCount = 0;
	}

	int getMaxFrameCount() { return maxFrameCount; }

	std::string getMaxItem() { return maxTimerName; }

	double getMaxFrameValue() { return maxTimeConsume; }

	~TimerManager() {}

	void startTimer(const TimerManager::Type t)
	{
		timers[t].start();
	}

	void stopTimer(const TimerManager::Type t)
	{
		timers[t].stop();
	}

	double getElapseTime(const TimerManager::Type t)
	{
		return timers[t].getElapsedTimeInMilliSec();
	}

	double getTotalElapsed()
	{
		return timers[0].getElapsedTimeInMilliSec();
	}

	void displayTimers(int x, int y)
	{
		//return;
		
		BWAPI::Broodwar->drawBoxScreen(x - 5, y - 5, x + 110 + barWidth, y + 5 + (10 * timers.size()), BWAPI::Colors::Black, true);
		BWAPI::Broodwar->drawTextScreen(x - 30, y - 70, "max frame time is : %.4f", maxTimeConsume);
		BWAPI::Broodwar->drawTextScreen(x - 30, y - 50, ">= 55ms count : %d", maxFrameCount);
		BWAPI::Broodwar->drawTextScreen(x - 30, y - 30, "             from : %s", maxTimerName.c_str());
		
		int yskip = 0;
		double total = timers[0].getElapsedTimeInMilliSec();
		for (size_t i(0); i < timers.size(); ++i)
		{
			double elapsed = timers[i].getElapsedTimeInMilliSec();

			if (elapsed > maxTimeConsume)
			{
				maxTimeConsume = elapsed;
			}

			if (elapsed > itemMaxTimeConsume && timerNames[i] != "Total")
			{
				itemMaxTimeConsume = elapsed;
				maxTimerName = timerNames[i];
			}

			if (timerNames[i] == "Total" && elapsed >= 55)
			{
				maxFrameCount += 1;
				if (maxFrameCount >= 321 && maxFrameCount <= 325) {
					stringstream ss;
					ss << elapsed;
					logInfo("TimeManager", "frame time=" + ss.str() + "ms (>=55ms), from " + maxTimerName, "BIG_ERROR_TimeManager");
				}
			}

			int width = (int)((elapsed == 0) ? 0 : (barWidth * (elapsed / total)));
			
			BWAPI::Broodwar->drawTextScreen(x, y + yskip - 3, "\x04 %s", timerNames[i].c_str());
			BWAPI::Broodwar->drawBoxScreen(x + 60, y + yskip, x + 60 + width + 1, y + yskip + 8, BWAPI::Colors::White);
			BWAPI::Broodwar->drawTextScreen(x + 70 + barWidth, y + yskip - 3, "%.4lf", elapsed);
			yskip += 10;
		}
	}

	static TimerManager& Instance()
	{
		static TimerManager a;
		return a;
	}
};
