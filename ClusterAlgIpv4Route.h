/*
 * ClusterAlgIpv4Route.h
 *
 *  Created on: Jul 17, 2020
 *      Author: ostro
 */

#ifndef INET_ROUTING_CLUSTER_ALG_CLUSTERALGIPV4ROUTE_H_
#define INET_ROUTING_CLUSTER_ALG_CLUSTERALGIPV4ROUTE_H_

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include <vector>
#include "inet/routing/cluster_alg/clusterAlgMessages_m.h"

namespace inet {

class NodeObject : public cObject
{
public:
    Ipv4Address address;
};


class INET_API ClusterAlgIpv4Route : public Ipv4Route
{
protected:
    simtime_t expiryTime;  // time the routing entry is valid until
public:
    unsigned int sequencenumber; // originated from destination. Ensures loop freeness.
    NodeState state;
    int acquaintanceCounter = 0;
    int undecidedNeighborsNum = -1;
    int distance;
    int neighborsNum;
    Ipv4Address clusterId;
    std::vector<Ipv4Address> neighborsClusterIds;
    std::vector<Ipv4Address> neighborsIds;

public:
    virtual bool isValid() const override
    {
        return expiryTime == 0 || expiryTime > simTime();
    }

    simtime_t getExpiryTime() const
    {
        return expiryTime;
    }
    void setExpiryTime(simtime_t time)
    {
        expiryTime = time;
    }

    void setSourceFromId(Ipv4Address id)
    {
        NodeObject *nObj = new NodeObject();
        nObj->address = id;
//        simtime_t expiryTime;  // time the routing entry is valid until
        setSource(nObj);
    }
    Ipv4Address getIdFromSource()
    {
        NodeObject *src = dynamic_cast<NodeObject*>(getSource());
        return src->address;
    }

};
}
#endif /* INET_ROUTING_CLUSTER_ALG_CLUSTERALGIPV4ROUTE_H_ */
