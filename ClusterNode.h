/*
 * ClusterNode.h
 *
 *  Created on: Jul 17, 2020
 *      Author: ostro
 */

#ifndef INET_ROUTING_CLUSTER_ALG_CLUSTERNODE_H_
#define INET_ROUTING_CLUSTER_ALG_CLUSTERNODE_H_

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

#include "inet/routing/cluster_alg/ClusterAlgIpv4Route.h"

namespace inet {

class ClusterInfo : public cObject
{
public:
    Ipv4Address clusterId;
    std::vector<Ipv4Address> members;
    simtime_t expiryTime;
    unsigned int seq;
    std::vector<Ipv4Address> neighborClusters;
    bool isForwarded = false;
};

class ClusterLink : public cTopology::Link
{
public:
    std::string relation = "";

    ClusterLink(double weight=1): cTopology::Link(weight){
    }

};

class ClusterNode : public cTopology::Node
{
public:
    ClusterInfo *clusterInfo = nullptr; //  from tc
    ClusterAlgIpv4Route *clusterRoute = nullptr; // from hello
    Ipv4Address address = Ipv4Address::UNSPECIFIED_ADDRESS; // self, direct and 2 hop neighbors

    ClusterNode(Ipv4Address address) :
            cTopology::Node(), address(address)
    {
    }

    ClusterNode(ClusterInfo *ci) :
            cTopology::Node(), clusterInfo(ci), clusterRoute(nullptr)
    {
    }

    ClusterNode(ClusterAlgIpv4Route *cr) :
            cTopology::Node(), clusterInfo(nullptr), clusterRoute(cr)
    {
    }

    static std::string getTextRepresentation(ClusterNode *node);
    static std::string toString(cTopology *topology);
    static std::string getLabel(ClusterLink *cl);
    static std::string getColor(ClusterLink *cl);

};
}
#endif /* INET_ROUTING_CLUSTER_ALG_CLUSTERNODE_H_ */
