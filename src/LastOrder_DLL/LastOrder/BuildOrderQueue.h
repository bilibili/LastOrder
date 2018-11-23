#pragma once

#include "Common.h"
#include "MetaType.h"

#define PRIORITY_TYPE int

template <class T>
class BuildOrderItem {

public:

	MetaType			metaType;		// the thing we want to 'build'
	T					priority;	// the priority at which to place it in the queue
	bool				blocking;	// whether or not we block further items
	std::vector<MetaType> waitingBuildType; //the subsequence items need build in the callback
	bool				hasAssignWorkerToMove;
	Unit				producer;

	BuildOrderItem(MetaType m, T p, bool b) : metaType(m), priority(p), blocking(b) 
	{
		waitingBuildType = std::vector<MetaType>();
		hasAssignWorkerToMove = false;
		producer = NULL;
	}

	BuildOrderItem(MetaType m, T p, bool b, std::vector<MetaType> w) : metaType(m), priority(p), blocking(b)
	{
		waitingBuildType = w;
		hasAssignWorkerToMove = false;
		producer = NULL;
	}

	bool operator<(const BuildOrderItem<T> &x) const
	{
		return priority < x.priority;
	}
};

class BuildOrderQueue {

public:
	std::deque< BuildOrderItem<PRIORITY_TYPE> >			queue;

	PRIORITY_TYPE lowestPriority;
	PRIORITY_TYPE highestPriority;
	PRIORITY_TYPE defaultPrioritySpacing;

	int numSkippedItems;


	BuildOrderQueue();

	void clearAll();											// clears the entire build order queue
	void clearAllUnit();
	void skipItem();											// increments skippedItems
	void queueAsHighestPriority(MetaType m, bool blocking, std::vector<MetaType> waitingBuildType = std::vector<MetaType>());		// queues something at the highest priority
	void queueAsLowestPriority(MetaType m, bool blocking, std::vector<MetaType> waitingBuildType = std::vector<MetaType>());		// queues something at the lowest priority
	void queueItem(MetaType m, bool blocking, PRIORITY_TYPE priority, std::vector<MetaType> waitingBuildType = std::vector<MetaType>());			// queues something with a given priority

	void removeHighestPriorityItem();								// removes the highest priority item
	void removeCurrentHighestPriorityItem();
	int removeUnitType(BWAPI::UnitType uType);
	void removeStrategyItem(std::string targetStrategy);

	int getHighestPriorityValue() { return highestPriority; }						// returns the highest priority value
	int	getLowestPriorityValue() { return lowestPriority; }								// returns the lowest priority value
	size_t size();													// returns the size of the queue

	bool isEmpty();

	void removeAll(MetaType m);									// removes all matching meta types from queue

	BuildOrderItem<PRIORITY_TYPE> & getHighestPriorityItem();	// returns the highest priority item
	BuildOrderItem<PRIORITY_TYPE> & getNextHighestPriorityItem();	// returns the highest priority item

	bool canSkipItem();
	bool hasNextHighestPriorityItem();								// returns the highest priority item

	void drawQueueInformation(int x, int y);
	bool isUpgradeInQueue(BWAPI::UpgradeType up);

	// overload the bracket operator for ease of use
	BuildOrderItem<PRIORITY_TYPE> operator [] (int i);
};