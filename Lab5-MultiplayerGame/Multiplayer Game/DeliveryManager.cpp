#include "Networks.h"
#include "DeliveryManager.h"


// TODO(Oscar): Reliability on top of UDP lab session

// Server
Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	uint32 sequenceNumber = nextSequenceNumber++;
	packet << sequenceNumber;
	Delivery* delivery = new Delivery(sequenceNumber);
	pendingDeliveries.push_back(delivery);

	return delivery;
}

//Server
// Here the server will receive a list of acks(uint32) till the end of the packet
void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	uint32 seq = 0;
	while (packet.RemainingByteCount() > 0)
	{
		// Get the next sequence number ack'd
		packet >> seq;
		// Shitty way to delete (?)
		for (int i = pendingDeliveries.size() - 1; i >= 0; i--)
		{
			if (pendingDeliveries[i]->sequenceNumber == seq)
			{
				if (pendingDeliveries[i]->delegate != nullptr)
					pendingDeliveries[i]->delegate->onDeliverySuccess(this);

				pendingDeliveries.erase(pendingDeliveries.begin() + i);
			}
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
	for (int i = pendingDeliveries.size() - 1; i >= 0; i--)
	{
		if (Time.time - pendingDeliveries[i]->dispatchTime >= PENDING_PACKETS_TIMEOUT_SECONDS)
		{
			if (pendingDeliveries[i]->delegate != nullptr)
				pendingDeliveries[i]->delegate->onDeliveryFailure(this);
			pendingDeliveries.erase(pendingDeliveries.begin() +i);
		}
	}
}

// Client
// Put to the list the sequence number recieved to later send the ack
bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 i = 0;
	packet >> i;
	if (i == nextExpectedSequenceNumber)
	{
		pendingAck.push_back(i);
		nextExpectedSequenceNumber++;
		return true;
	}


	return false;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	return !pendingAck.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	while (!pendingAck.empty())
	{
		packet << pendingAck.front();
		pendingAck.pop_front();
	}
}

