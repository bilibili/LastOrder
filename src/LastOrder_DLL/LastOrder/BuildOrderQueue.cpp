
#include "BuildOrderQueue.h"

BuildOrderQueue::BuildOrderQueue()
	: highestPriority(0),
	lowestPriority(0),
	defaultPrioritySpacing(10),
	numSkippedItems(0)
{

}

bool BuildOrderQueue::isUpgradeInQueue(BWAPI::UpgradeType up)
{
	for (auto i : queue)
	{
		if (i.metaType.upgradeType == up)
		{
			return true;
		}
	}
	return false;
}

void BuildOrderQueue::clearAll()
{
	// clear the queue
	queue.clear();

	// reset the priorities
	highestPriority = 0;
	lowestPriority = 0;
}

void BuildOrderQueue::clearAllUnit()
{
	for (std::deque< BuildOrderItem<PRIORITY_TYPE>>::iterator it = queue.begin(); it != queue.end();)
	{
		if (it->metaType.isBuilding() || it->metaType.unitType == BWAPI::UnitTypes::Zerg_Drone)
		{
			it++;
		}
		else
			it = queue.erase(it);
	}
}

BuildOrderItem<PRIORITY_TYPE> & BuildOrderQueue::getHighestPriorityItem()
{
	// reset the number of skipped items to zero
	numSkippedItems = 0;

	// the queue will be sorted with the highest priority at the back
	return queue.back();
}

BuildOrderItem<PRIORITY_TYPE> & BuildOrderQueue::getNextHighestPriorityItem()
{
	assert(queue.size() - 1 - numSkippedItems >= 0);

	// the queue will be sorted with the highest priority at the back
	return queue[queue.size() - 1 - numSkippedItems];
}

void BuildOrderQueue::skipItem()
{
	// make sure we can skip
	assert(canSkipItem());

	// skip it
	numSkippedItems++;
}



bool BuildOrderQueue::canSkipItem() {

	// does the queue have more elements
	bool bigEnough = queue.size() > (size_t)(1 + numSkippedItems);

	if (!bigEnough)
	{
		return false;
	}

	// is the current highest priority item not blocking a skip
	bool highestNotBlocking = !queue[queue.size() - 1 - numSkippedItems].blocking;

	//if (highestNotBlocking)
		//BWAPI::Broodwar->printf("skip unit %s", queue[queue.size() - 1 - numSkippedItems].metaType.unitType.getName().c_str());

	// this tells us if we can skip
	return highestNotBlocking;
}

void BuildOrderQueue::queueItem(MetaType m, bool blocking, PRIORITY_TYPE priority, std::vector<MetaType> waitingBuildType)
{
	// if the queue is empty, set the highest and lowest priorities
	if (queue.empty())
	{
		highestPriority = priority;
		lowestPriority = priority;
	}

	// push the item into the queue
	if (priority <= lowestPriority)
	{
		queue.push_front(BuildOrderItem<PRIORITY_TYPE>(m, priority, blocking, waitingBuildType));
	}
	else
	{
		queue.push_back(BuildOrderItem<PRIORITY_TYPE>(m, priority, blocking, waitingBuildType));
	}

	// if the item is somewhere in the middle, we have to sort again
	if ((queue.size() > 1) && (priority < highestPriority) && (priority > lowestPriority))
	{
		// sort the list in ascending order, putting highest priority at the top
		std::sort(queue.begin(), queue.end());
	}

	// update the highest or lowest if it is beaten
	highestPriority = (priority > highestPriority) ? priority : highestPriority;
	lowestPriority = (priority < lowestPriority) ? priority : lowestPriority;
}

void BuildOrderQueue::queueAsHighestPriority(MetaType m, bool blocking, std::vector<MetaType> waitingBuildType)
{
	// the new priority will be higher
	PRIORITY_TYPE newPriority = highestPriority + defaultPrioritySpacing;

	// queue the item
	queueItem(m, blocking, newPriority, waitingBuildType);
}

void BuildOrderQueue::queueAsLowestPriority(MetaType m, bool blocking, std::vector<MetaType> waitingBuildType)
{
	// the new priority will be higher
	int newPriority = lowestPriority - defaultPrioritySpacing;

	// queue the item
	queueItem(m, blocking, newPriority, waitingBuildType);
}


int BuildOrderQueue::removeUnitType(BWAPI::UnitType uType)
{
	for (std::deque< BuildOrderItem<PRIORITY_TYPE>>::iterator it = queue.begin(); it != queue.end();it++)
	{
		if (it->metaType.unitType == uType)
		{
			it = queue.erase(it);
			return 1;
		}
	}
	return 0;
}


void BuildOrderQueue::removeStrategyItem(std::string targetStrategy)
{
	for (std::deque< BuildOrderItem<PRIORITY_TYPE>>::iterator it = queue.begin(); it != queue.end();)
	{
		if (it->metaType.unitSourceBuildingAction == targetStrategy)
		{
			it = queue.erase(it);
		}
		else
		{
			it++;
		}
	}

}



void BuildOrderQueue::removeHighestPriorityItem()
{
	// remove the back element of the vector
	queue.pop_back();

	// if the list is not empty, set the highest accordingly
	highestPriority = queue.empty() ? 0 : queue.back().priority;
	lowestPriority = queue.empty() ? 0 : lowestPriority;
}

void BuildOrderQueue::removeCurrentHighestPriorityItem()
{
	// remove the back element of the vector
	queue.erase(queue.begin() + queue.size() - 1 - numSkippedItems);

	//assert((int)(queue.size()) < size);

	// if the list is not empty, set the highest accordingly
	highestPriority = queue.empty() ? 0 : queue.back().priority;
	lowestPriority = queue.empty() ? 0 : lowestPriority;
}

size_t BuildOrderQueue::size()
{
	return queue.size();
}

bool BuildOrderQueue::isEmpty()
{
	return (queue.size() == 0);
}

BuildOrderItem<PRIORITY_TYPE> BuildOrderQueue::operator [] (int i)
{
	return queue[i];
}

void BuildOrderQueue::drawQueueInformation(int x, int y) {

	//x = x + 25;

	std::string prefix = "\x04";

	//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04Priority Queue Information:");
	//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04UNIT NAME");

	size_t reps = queue.size() < 10 ? queue.size() : 10;

	// for each unit in the queue
	for (size_t i(0); i < reps; i++) {

		prefix = "\x04";

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y + (i * 10), "%s%s", prefix.c_str(), queue[queue.size() - 1 - i].metaType.getName().c_str());
	}
}
