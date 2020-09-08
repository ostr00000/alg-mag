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

#ifndef INET_ROUTING_CLUSTER_ALG_RANDOMORDERPINGAPP_H_
#define INET_ROUTING_CLUSTER_ALG_RANDOMORDERPINGAPP_H_

#include "inet/applications/pingapp/PingApp.h"
#include "inet/networklayer/common/HopLimitTag_m.h"

namespace inet {

class RandomOrderPingApp: public PingApp
{
    simsignal_t hopNumberSignal;

protected:
    void initialize(int stage) override;
    void handleSelfMessage(cMessage *msg) override;
    std::vector<L3Address> getAllAddresses() override;
    void processPingResponse(int originatorId, int seqNo, Packet *packet) override;
};

} /* namespace inet */

#endif /* INET_ROUTING_CLUSTER_ALG_RANDOMORDERPINGAPP_H_ */
