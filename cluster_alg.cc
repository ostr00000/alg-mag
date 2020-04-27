#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/routing/cluster_alg/cluster_alg.h"

namespace inet {

Define_Module(ClusterAlg);

ClusterAlg::ClusterAlg()
{
}

ClusterAlg::~ClusterAlg()
{
    stop();
    // Dispose of dynamically allocated the objects
    delete event;
    //delete Hello;
}

void ClusterAlg::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    //reads from omnetpp.ini
    if (stage == INITSTAGE_LOCAL) {
        sequencenumber = 0;
        host = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);

        routeLifetime = par("routeLifetime").doubleValue();
        helloInterval = par("helloInterval");
        event = new cMessage("event");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::manet, nullptr, gate("ipIn"));
        registerProtocol(Protocol::manet, gate("ipOut"), nullptr);
    }
}

void ClusterAlg::start()
{
    /* Search the 80211 interface */
    int num_80211 = 0;
    InterfaceEntry *ie;
    InterfaceEntry *i_face;
    const char *name;
    broadcastDelay = &par("broadcastDelay");
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        ie = ift->getInterface(i);
        name = ie->getInterfaceName();
        if (strstr(name, "wlan") != nullptr) {
            i_face = ie;
            num_80211++;
            interfaceId = i;
        }
    }

    // One enabled network interface (in total)
    if (num_80211 == 1)
        interface80211ptr = i_face;
    else
        throw cRuntimeError("ClusterAlg has found %i 80211 interfaces", num_80211);
    if (par("manetPurgeRoutingTables").boolValue()) {
        Ipv4Route *entry;
        // clean the route table wlan interface entry
        for (int i = rt->getNumRoutes() - 1; i >= 0; i--) {
            entry = rt->getRoute(i);
            const InterfaceEntry *ie = entry->getInterface();
            if (strstr(ie->getInterfaceName(), "wlan") != nullptr)
                rt->deleteRoute(entry);
        }
    }
    CHK(interface80211ptr->getProtocolData<Ipv4InterfaceData>())->joinMulticastGroup(Ipv4Address::LL_MANET_ROUTERS);

    // schedules a random periodic event: the hello message broadcast from ClusterAlg module

    //reads from omnetpp.ini
    //HelloForward = new ClusterAlgHello("HelloForward");
    // schedules a random periodic event: the hello message broadcast from ClusterAlg module
    scheduleAt(simTime() + uniform(0.0, par("maxVariance").doubleValue()), event);
    myState = NodeState::UNDECIDED;
    clusterId = Ipv4Address::UNSPECIFIED_ADDRESS;
}

void ClusterAlg::stop()
{
    cancelEvent(event);
}

void ClusterAlg::handleSelfMessage(cMessage *msg)
{
    if (msg == event) {
        auto hello = makeShared<ClusterAlgHello>();
        rt->purge(); // remove invalid ipv4route

        Ipv4Address source = interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
        hello->setChunkLength(b(128)); ///size of Hello message in bits
        hello->setSrcAddress(source);
        sequencenumber += 2;
        hello->setSequencenumber(sequencenumber);
        hello->setNextAddress(source);
        hello->setHopdistance(1);

        hello->setMessageType(HELLO);
        hello->setState(myState);
        hello->setSrcId(interface80211ptr->getIpv4Address());
        hello->setClusterHeadId(clusterId);

        std::list<Ipv4Address> oneHopNeighbors;
        for (int k = 0, total = rt->getNumRoutes(); k < total; k++) {
            ClusterAlgIpv4Route *route = dynamic_cast<ClusterAlgIpv4Route*>(rt->getRoute(k));
            if (route != nullptr && route->getMetric() == 1) {
                oneHopNeighbors.push_back(route->getDestination());
            }
        }
        hello->setNeighborsArraySize(oneHopNeighbors.size());
        int index = 0;
        for (auto const &n : oneHopNeighbors) {
            hello->setNeighbors(index, n);
            index += 1;
        }

        //new control info for ClusterAlgHello
        auto packet = new Packet("Hello", hello);
        auto addressReq = packet->addTag<L3AddressReq>();
        addressReq->setDestAddress(Ipv4Address(255, 255, 255, 255));
        addressReq->setSrcAddress(source);
        packet->addTag<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

        //broadcast to other nodes the hello message
        send(packet, "ipOut");
        packet = nullptr;
        hello = nullptr;

        //schedule new broadcast hello message event
        scheduleAt(simTime() + helloInterval + broadcastDelay->doubleValue(), event);
        bubble("Sending new hello message");
    }
}

void ClusterAlg::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
        return;
    }
    else if (check_and_cast<Packet*>(msg)->getTag<PacketProtocolTag>()->getProtocol() != &Protocol::manet) {
        throw cRuntimeError("Message not supported %s", msg->getName());
    }

    if (!msg->arrivedOn("ipIn")) {
        throw cRuntimeError("Message arrived on unknown gate %s", msg->getArrivalGate()->getName());
    }

    auto type = staticPtrCast<ClusterAlgBase>(check_and_cast<Packet*>(msg)->peekData<ClusterAlgBase>()->dupShared())->getMessageType();

    if (type == MessageType::HELLO) {
        auto recHello = staticPtrCast<ClusterAlgHello>(check_and_cast<Packet*>(msg)->peekData<ClusterAlgHello>()->dupShared());
        receiveHello(recHello);

    }
    else if (type == MessageType::TOPOLOGY_CONTROL) {
        auto recTopolgyControl = staticPtrCast<ClusterAlgTopologyControl>(
                check_and_cast<Packet*>(msg)->peekData<ClusterAlgTopologyControl>()->dupShared());
        receiveTopologyControl(recTopolgyControl);

    }
    else {
        throw cRuntimeError("Unknown message type");
    }

    delete msg;
}

void ClusterAlg::receiveHello(IntrusivePtr<inet::ClusterAlgHello> &recHello)
{
    bubble("Received hello message");
    Ipv4Address myIp = interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress();

    Ipv4Address src = recHello->getSrcAddress();
    Ipv4Address next = recHello->getNextAddress();
    unsigned int msgsequencenumber = recHello->getSequencenumber();
    int numHops = recHello->getHopdistance();

    Ipv4Route *route = rt->findBestMatchingRoute(src);
    ClusterAlgIpv4Route *clusterAlgRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);

    if (noRoute(route) || isNotBroadcast(route) || hasBetterSeqNumber(clusterAlgRoute, sequencenumber)
            || hasSameSeqNumButShortestPath(clusterAlgRoute, sequencenumber, numHops)) {
        removeOldRoute(clusterAlgRoute);
        addNewRoute(src, next, numHops, msgsequencenumber);
    }
}

inline bool ClusterAlg::noRoute(Ipv4Route *route)
{
    return route == nullptr;
}
inline bool ClusterAlg::isNotBroadcast(Ipv4Route *route)
{
    return route != nullptr and route->getNetmask() != Ipv4Address::ALLONES_ADDRESS;
}
inline bool ClusterAlg::hasBetterSeqNumber(ClusterAlgIpv4Route *route, unsigned int seqNum)
{
    return route != nullptr and route->getSequencenumber() < seqNum;
}
inline bool ClusterAlg::hasSameSeqNumButShortestPath(ClusterAlgIpv4Route *route, unsigned int seqNum, int hopsNum)
{
    return route != nullptr and route->getSequencenumber() == seqNum and route->getMetric() == hopsNum;
}
inline bool ClusterAlg::removeOldRoute(ClusterAlgIpv4Route *route)
{
    if (route != nullptr) {
        rt->deleteRoute(route);
    }
}

void ClusterAlg::addNewRoute(Ipv4Address src, Ipv4Address next, int metric, unsigned int msgSeq)
{
    ClusterAlgIpv4Route *e = new ClusterAlgIpv4Route();
    e->setDestination(src);
    e->setNetmask(Ipv4Address::ALLONES_ADDRESS);
    e->setGateway(next);
    e->setInterface(interface80211ptr);
    e->setSourceType(IRoute::MANET);
    e->setMetric(metric);
    e->setSequencenumber(msgSeq);
    e->setExpiryTime(simTime() + routeLifetime);
    rt->addRoute(e);
}

void ClusterAlg::receiveTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{

}

} // namespace inet

