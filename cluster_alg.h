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
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/cluster_alg/clusterAlgMessages_m.h"

namespace inet {

class INET_API ClusterAlgIpv4Route : public Ipv4Route
{
    protected:
        unsigned int sequencenumber; // originated from destination. Ensures loop freeness.
        simtime_t expiryTime;  // time the routing entry is valid until

    public:
        virtual bool isValid() const override { return expiryTime == 0 || expiryTime > simTime(); }

        simtime_t getExpiryTime() const {return expiryTime;}
        void setExpiryTime(simtime_t time) {expiryTime = time;}
        void setSequencenumber(int i) {sequencenumber = i;}
        unsigned int getSequencenumber() const {return sequencenumber;}
};



class INET_API ClusterAlg : public RoutingProtocolBase
{
  private:
    cMessage *event = nullptr;
    cPar *broadcastDelay = nullptr;
    InterfaceEntry *interface80211ptr = nullptr;
    int interfaceId = -1;
    unsigned int sequencenumber = 0;
    simtime_t routeLifetime;
    cModule *host = nullptr;

    NodeState myState;
    Ipv4Address clusterId;
    std::list<Ipv4Address> neighbors;

  protected:
    simtime_t helloInterval;
    IInterfaceTable *ift = nullptr;
    IIpv4RoutingTable *rt = nullptr;

  public:
    ClusterAlg();
    ~ClusterAlg();

  protected:
    void receiveHello(IntrusivePtr<inet::ClusterAlgHello>& recHello);
    void receiveTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl>& topologyControl);
    void addNewRoute(Ipv4Address src, Ipv4Address next, int metric, unsigned int msgSeq);
    inline bool noRoute(Ipv4Route *route);
    inline bool isNotBroadcast(Ipv4Route *route);
    inline bool hasBetterSeqNumber(ClusterAlgIpv4Route *route, unsigned int seqNum);
    inline bool hasSameSeqNumButShortestPath(ClusterAlgIpv4Route *route, unsigned int seqNum, int hopsNum);
    inline bool removeOldRoute(ClusterAlgIpv4Route *route);


    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void handleSelfMessage(cMessage *msg);

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override { start(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { stop(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override  { stop(); }
    void start();
    void stop();
};


} // namespace inet

#endif // ifndef __INET_CLUSTER_ALG_H

