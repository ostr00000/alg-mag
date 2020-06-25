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
        simtime_t expiryTime;  // time the routing entry is valid until

        setSource(nObj);
    }
    Ipv4Address getIdFromSource()
    {
        NodeObject *src = dynamic_cast<NodeObject*>(getSource());
        return src->address;
    }

};

class ClusterInfo : public cObject
{
public:
    Ipv4Address clusterId;
    std::vector<Ipv4Address> members;
    simtime_t expiryTime;
    unsigned int seq;
    std::vector<Ipv4Address> neighborClusters;
};

class ClusterNode : public cTopology::Node
{
public:
    ClusterInfo *clusterInfo;
    ClusterAlgIpv4Route *clusterRoute;

    ClusterNode(ClusterInfo *ci) :
            cTopology::Node(), clusterInfo(ci), clusterRoute(nullptr)
    {
    }

    ClusterNode(ClusterAlgIpv4Route *cr) :
            cTopology::Node(), clusterInfo(nullptr), clusterRoute(cr)
    {
    }

};

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
    cMessage *communicationMessageEvent = nullptr;

    cPar *broadcastDelay = nullptr;
    InterfaceEntry *interface80211ptr = nullptr;
    int interfaceId = -1;
    unsigned int sequencenumber = 0;
    unsigned int topologySeq = 0;
    simtime_t routeLifetime;
    cModule *host = nullptr;

    Ipv4Address myIp;
public:
    NodeState myState;
    Ipv4Address clusterId;
    std::map<Ipv4Address, int> counterOfSeenNeighborsLeaders;

    std::map<Ipv4Address, ClusterInfo*> addressToCluster;
    std::map<Ipv4Address, ClusterNode*> clusterIdToNode;


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
    inline void removeOldRoute(ClusterAlgIpv4Route *route);

    void handleHelloEvent();
    void handleTopolgyEvent();
    void handleCommunicationMessageEvent();

    void forwardTC(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl, bool resetForwardNodes);
    void setAllowedToForwardNodes(IntrusivePtr<inet::ClusterAlgTopologyControl> &tc);
    void setNeighborsCluster(IntrusivePtr<inet::ClusterAlgTopologyControl> &tc);
    bool updateTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl);
    void scheduleTopologyControl(simtime_t scheduleTime);
    void handleClusterStateEvent();
    void refreshTextFromState();

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
    virtual Result datagramPreRoutingHook(Packet *datagram) override;
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
        return ACCEPT;
    }
    void delayDatagram(Packet *datagram);

};

} // namespace inet

#endif // ifndef __INET_CLUSTER_ALG_H

