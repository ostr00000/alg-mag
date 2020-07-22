#ifndef __INET_CLUSTER_ALG_H
#define __INET_CLUSTER_ALG_H

#include <list>
#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>

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
#include "inet/routing/cluster_alg/clusterAlgMessages_m.h"
#include "inet/routing/cluster_alg/ClusterNode.h"
namespace inet {


class INET_API ClusterAlg : public RoutingProtocolBase, public NetfilterBase::HookBase
{
private:
    simsignal_t helloSignal;
    simsignal_t topologyControlSignal;
    simsignal_t stateChangedSignal;
    simsignal_t clusterDestroyedSignal;
    simsignal_t messageSendSignal;
    simsignal_t messageTransferedSignal;
    simsignal_t messageReceivedSignal;

    cMessage *helloEvent = nullptr;
    cMessage *tcEvent = nullptr;
    cMessage *clusterStateEvent = nullptr;
    cMessage *topolgyControlEvent = nullptr;

    cPar *broadcastDelay = nullptr;
    InterfaceEntry *interface80211ptr = nullptr;
    int interfaceId = -1;
    unsigned int sequencenumber = 0;
    unsigned int topologySeq = 0;
    simtime_t routeLifetime;
    cModule *host = nullptr;

    Ipv4Address myIp;
    int addressToClusterSize = 0;
public:
    NodeState myState;
    Ipv4Address clusterId;
    std::map<Ipv4Address, int> counterOfSeenNeighborsLeaders;

    std::map<Ipv4Address, ClusterInfo*> addressToCluster;
    std::map<Ipv4Address, ClusterNode*> idToNode;
    ClusterNode *myNode = nullptr;

protected:
    simtime_t helloInterval;
    simtime_t tcInterval;
    IInterfaceTable *ift = nullptr;
    IIpv4RoutingTable *rt = nullptr;
    cTopology *clusterGraph = nullptr;
    INetfilter *networkProtocol = nullptr;

public:
    ClusterAlg();
    ~ClusterAlg();

protected:
    void receiveHello(IntrusivePtr<inet::ClusterAlgHello> &recHello);
    void receiveTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl);
    ClusterAlgIpv4Route* addNewRoute(Ipv4Address dest, Ipv4Address next, Ipv4Address source, int distance,
            IntrusivePtr<inet::ClusterAlgHello> &recHello);
    ClusterAlgIpv4Route* addNewRouteToClusterLeader(Ipv4Address dest, Ipv4Address clusterId, int distance);
    ClusterAlgIpv4Route* addNewRoute(Ipv4Address dest, Ipv4Address gateway, Ipv4Address clusterId, int distance, NodeState state);


    inline void removeOldRoute(ClusterAlgIpv4Route *route);

    void handleHelloEvent();
    void handleTopolgyEvent();

    void forwardTC(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl, bool resetForwardNodes);
    void setAllowedToForwardNodes(IntrusivePtr<inet::ClusterAlgTopologyControl> &tc);
    ClusterAlgIpv4Route* findBestCandidateToForward(Ipv4Address clusterId, std::multimap<Ipv4Address, ClusterAlgIpv4Route*> clusterToNode);
    void setNeighborsCluster(IntrusivePtr<inet::ClusterAlgTopologyControl> &tc);
    bool updateTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl);
    void scheduleTopologyControl(simtime_t scheduleTime);
    void handleClusterStateEvent();
    void refreshTextFromState();

    Ipv4Address calculateRoute(Ipv4Address address);
    void recomputeRoute();

    virtual int numInitStages() const override
    {
        return NUM_INIT_STAGES;
    }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void handleSelfMessage(cMessage *msg);

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override
    {
        start();
    }
    virtual void handleStopOperation(LifecycleOperation *operation) override
    {
        stop();
    }
    virtual void handleCrashOperation(LifecycleOperation *operation) override
    {
        stop();
    }
    void start();
    void stop();

    /* Netfilter hooks */
    Result ensureRouteForDatagram(Packet *datagram);

    virtual Result datagramPreRoutingHook(Packet *datagram) override
    {
        return ensureRouteForDatagram(datagram);
    }
    virtual Result datagramForwardHook(Packet *datagram) override
    {
        return ACCEPT;
    }
    virtual Result datagramPostRoutingHook(Packet *datagram) override
    {
        return ACCEPT;
    }
    virtual Result datagramLocalInHook(Packet *datagram) override
    {
        return ACCEPT;
    }
    virtual Result datagramLocalOutHook(Packet *datagram) override
    {
        return ensureRouteForDatagram(datagram);
    }

};

} // namespace inet

#endif // ifndef __INET_CLUSTER_ALG_H

