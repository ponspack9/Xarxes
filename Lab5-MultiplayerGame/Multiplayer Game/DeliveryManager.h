#pragma once
#include "Networks.h"
#include <vector>
#include <list>
#include "ReplicationCommand.h"
// TODO(Oscar): Reliability on top of UDP lab session

// REPLICATION_ID
// Sequence number
// Replication_type
// Replication_info
// ...
// Replication_type
// Replication_info

class DeliveryManager;

class DeliveryDelegate
{
public:

	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	Delivery(uint32 sequenceNumber)
	{
		this->sequenceNumber = sequenceNumber;
		dispatchTime = Time.time;
		delegate = nullptr;
	}
	
	uint32 sequenceNumber = 0;
	real64 dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
	
	std::vector<uint32> networkIds;
	std::vector < ReplicationAction> actions;
};

class DeliveryManager
{
public:

	// For senders to write a new sequence number into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	// For senders to process acknowledged sequence numbers from a packet
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();

	// For receivers to process the sequence number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream& packet);

	// For receivers to write acknowloedged sequence numbers into a packet
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);


	//void clear();
// Made public to show them in gui
public:

	/*struct DeliveryComparator
	{
		bool operator()(const Delivery* del1,const Delivery* del2) const
		{
			return del1->sequenceNumber < del2->sequenceNumber;
		}
	};*/
	// sender side /SERVER
	// The next outgoing sequence number
	uint32 nextSequenceNumber = 0;
	// A list of pending deliveries where index is equal to the sequenceNumber
	//Delivery pendingDeliveries[MAX_PENDING_PACKETS];
	std::vector<Delivery*> pendingDeliveries;

	// Reciever side / CLIENT
	// The next expected sequence number
	uint32 nextExpectedSequenceNumber = 0;
	// Alist of sequence numbers pending ack
	//uint32 pendingAck[MAX_PENDING_PACKETS];
	std::list<uint32> pendingAck;

};