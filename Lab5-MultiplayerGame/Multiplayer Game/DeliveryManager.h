#pragma once
#include "Networks.h"

// TODO(Oscar): Reliability on top of UDP lab session

class DeliveryManager;

class DeliveryDelegate
{
public:

	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	uint32 sequenceNumber = 0;
	real64 dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager
{
public:

	// For senders to write a new sequence number into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	// For receivers to process the sequence number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream& packet);

	// For receivers to write acknowloedged sequence numbers into a packet
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	// For senders to process acknowledged sequence numbers from a packet
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();

	void clear();

private:
	// sender side
	// The next outgoing sequence number
	uint32 nextSequenceNumber = 0;
	// A list of pending deliveries
	Delivery pendingDeliveries[MAX_PENDING_PACKETS];

	// Reciever side
	// The nect expected sequence number
	uint32 nextExpectedSequenceNumber = 0;
	// Alist of sequence numbers pending ack
	uint32 pendingAck[MAX_PENDING_PACKETS];

};