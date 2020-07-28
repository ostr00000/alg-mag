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

#include "RandomOrderPingApp.h"
#include <algorithm>
#include "inet/applications/pingapp/PingApp_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(RandomOrderPingApp);

void RandomOrderPingApp::handleSelfMessage(cMessage *msg)
{
    if (msg->getKind() == PING_CHANGE_ADDR) {
        destAddrIdx = this->intrand(destAddresses.size());
    }
    PingApp::handleSelfMessage(msg);
}

std::vector<L3Address> RandomOrderPingApp::getAllAddresses()
{
    auto allAddresses = PingApp::getAllAddresses();
    std::random_shuffle(allAddresses.begin(), allAddresses.end());
    return allAddresses;
}

} /* namespace inet */
