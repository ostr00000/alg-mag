//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "QueueSequentialTransmission.h"

namespace inet {

Define_Module(QueueSequentialTransmission);

QueueSequentialTransmission::QueueSequentialTransmission()
{

}

QueueSequentialTransmission::~QueueSequentialTransmission()
{
    if (forwardEvent != nullptr) {
        cancelEvent(forwardEvent);
        delete forwardEvent;
    }
}

void QueueSequentialTransmission::initialize()
{
    forwardEvent = new cMessage("forwardEvent");
}

void QueueSequentialTransmission::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        sendQueuedMessage();
    }
    else {
        this->myQueue.push(msg);
        sendQueuedMessage();
    }
}

void QueueSequentialTransmission::sendQueuedMessage()
{
    if (forwardEvent->isScheduled()) {
        return;  // sending is already planned
    }

    simtime_t transmissionFinish = getTransmissionFinishTime();
    if (transmissionFinish > 0) {
        scheduleAt(simTime() + transmissionFinish, forwardEvent); // need to wait a bit
        return;
    }

    if (!myQueue.empty()) {
        cMessage *msg = myQueue.front();
        myQueue.pop();
        send(msg, "out");

        if (!myQueue.empty()) { // there are more messages to send
            transmissionFinish = getTransmissionFinishTime();
            scheduleAt(simTime() + transmissionFinish, forwardEvent);
        }
    }
}

simtime_t QueueSequentialTransmission::getTransmissionFinishTime()
{
    cGate *gateOut = gate("out");
    cChannel *channel = gateOut->getChannel();
    simtime_t endTransmissionTime = channel->getTransmissionFinishTime();
    simtime_t now = simTime();

    simtime_t delayToWait = 0;
    if (endTransmissionTime > now) {
        delayToWait = endTransmissionTime - now;
    }
    return delayToWait;
}

} //namespace
