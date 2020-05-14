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
    delete helloEvent;
    delete clusterStateEvent;
    delete topolgyControlEvent;
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
        helloEvent = new cMessage("helloEvent");
        clusterStateEvent = new cMessage("clusterStateEvent");
        topolgyControlEvent = new cMessage("topolgyControlEvent");

        WATCH(myState);
        WATCH(clusterId);
        WATCH(myIp);

        helloSignal = registerSignal("helloSignal");
        topologyControlSignal = registerSignal("topologyControlSignal");
        stateChangedSignal = registerSignal("stateChangedSignal");
        clusterDestroyedSignal = registerSignal("clusterDestroyedSignal");

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

    // schedules a random periodic helloEvent: the hello message broadcast from ClusterAlg module

    //reads from omnetpp.ini
    //HelloForward = new ClusterAlgHello("HelloForward");
    // schedules a random periodic helloEvent: the hello message broadcast from ClusterAlg module
    scheduleAt(simTime() + uniform(0.0, par("maxVariance").doubleValue()), helloEvent);
    myState = NodeState::UNDECIDED;
    clusterId = Ipv4Address::UNSPECIFIED_ADDRESS;
    myIp = interface80211ptr->getIpv4Address();

    scheduleAt(simTime() + par("clusterUndecidedFirstTime").doubleValue() + uniform(0.0, par("maxVariance").doubleValue()), clusterStateEvent);

}

void ClusterAlg::stop()
{
    cancelEvent(helloEvent);
    cancelEvent(clusterStateEvent);
}

void ClusterAlg::handleSelfMessage(cMessage *msg)
{
    if (msg == helloEvent) {
        handleHelloEvent();
    }
    else if (msg == clusterStateEvent) {
        NodeState stateBefore = myState;

        handleClusterStateEvent();
        refreshTextFromState();

        if (stateBefore != myState){
            emit(stateChangedSignal, 1);
            if (stateBefore == NodeState::LEADER){
                emit(clusterDestroyedSignal, 1);
            }
        }
    }
}

void ClusterAlg::refreshTextFromState()
{
    char label[50];
    char color[10] = "#RRGGBB";

    std::hash<std::string> h;
    int c = h(clusterId.str(true));
    std::stringstream sstream;
    sstream << std::hex << c;
    std::string result = sstream.str() + "1234567";
    for (int i = 1; i <= 6; i++) {
        color[i] = result[i];
    }

    if (myState == NodeState::UNDECIDED) {
        sprintf(label, "Undecided");
    }
    else if (myState == NodeState::LEADER) {
        sprintf(label, "Leader [%d]", clusterId.getInt());
    }
    else if (myState == NodeState::MEMBER) {
        sprintf(label, "Member [%d]", clusterId.getInt());
    }

    getParentModule()->getDisplayString().setTagArg("t", 0, label);
    getParentModule()->getDisplayString().setTagArg("t", 2, color);
}

void ClusterAlg::handleHelloEvent()
{
    auto hello = makeShared<ClusterAlgHello>();

    //        Ipv4Address source = interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
    hello->setChunkLength(b(128)); ///size of Hello message in bits
    sequencenumber += 2;
    hello->setSequencenumber(sequencenumber);
    hello->setHopdistance(1);
    hello->setUndecidedNeighborsNum(operationalState);

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
    addressReq->setSrcAddress(interface80211ptr->getIpv4Address());
    packet->addTag<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

    //broadcast to other nodes the hello message
    send(packet, "ipOut");
    packet = nullptr;
    hello = nullptr;

    //schedule new broadcast hello message helloEvent
    scheduleAt(simTime() + helloInterval + broadcastDelay->doubleValue(), helloEvent);
    bubble("Sending new hello message");
}

void ClusterAlg::handleClusterStateEvent()
{
    if (myState == NodeState::UNDECIDED) {
        int myUndecidedDirectNeighbors = 0;
        int neighborBiggestUndecidedNeighbors = -1;
        Ipv4Address idUndecided = Ipv4Address::UNSPECIFIED_ADDRESS;

        // find any leader
        for (int i = 0, j = rt->getNumRoutes(); i < j; i++) {
            Ipv4Route *route = rt->getRoute(i);
            ClusterAlgIpv4Route *clusterRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);
            if (clusterRoute == nullptr) {
                continue;
            }
            // if isDirectNeighbor and isLeader
            if (clusterRoute->distance == 1) {
                if (clusterRoute->state == NodeState::LEADER) {
                    myState = NodeState::MEMBER;
                    clusterId = clusterRoute->getGateway();
                    scheduleAt(simTime() + par("clusterMember").doubleValue(), clusterStateEvent);
                    bubble(("Become member of " + clusterId.str()).c_str());

                    cancelEvent(helloEvent);
                    scheduleAt(simTime(), helloEvent);
                    return;

                }
                else if (clusterRoute->state == NodeState::UNDECIDED) {
                    myUndecidedDirectNeighbors += 1;

                    // if hasMoreUndecidedNeighbors or sameNumberButHigherId
                    if (clusterRoute->undecidedNeighborsNum > neighborBiggestUndecidedNeighbors
                            || (clusterRoute->undecidedNeighborsNum == neighborBiggestUndecidedNeighbors && clusterRoute->getGateway() > idUndecided)) {
                        neighborBiggestUndecidedNeighbors = clusterRoute->undecidedNeighborsNum;
                        idUndecided = clusterRoute->getGateway();
                    }
                }
            }
        }

        // become leader if number of undecided direct neighbors is greatest along all neighbors
        if (neighborBiggestUndecidedNeighbors < myUndecidedDirectNeighbors
                || (neighborBiggestUndecidedNeighbors == myUndecidedDirectNeighbors && idUndecided > interface80211ptr->getIpv4Address())) {
            myState = NodeState::LEADER;
            clusterId = interface80211ptr->getIpv4Address();
            scheduleAt(simTime() + par("clusterLeader").doubleValue(), clusterStateEvent);
            bubble(("Become leader " + clusterId.str()).c_str());

            cancelEvent(helloEvent);
            scheduleAt(simTime(), helloEvent); // TODO topology message too

            return;
        }

        scheduleAt(simTime() + par("clusterUndecided").doubleValue() + uniform(0.0, par("maxVariance").doubleValue()), clusterStateEvent);

    }
    else if (myState == NodeState::MEMBER) {
        bool myLeaderIsInRange = false;

        //check if current leader is in range
        for (int i = 0, j = rt->getNumRoutes(); i < j; i++) {
            Ipv4Route *route = rt->getRoute(i);
            ClusterAlgIpv4Route *clusterRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);
            if (clusterRoute == nullptr) {
                continue;
            }
            // if isDirectNeighbor and isLeader
            if (clusterRoute->distance == 1) {
                if (clusterRoute->state == NodeState::LEADER) {
                    if (clusterId == clusterRoute->getGateway()) {
                        myLeaderIsInRange = true;
                        break;
                    }
                }
            }
        }

        if (myLeaderIsInRange) {
            scheduleAt(simTime() + par("clusterMember").doubleValue(), clusterStateEvent);
        }
        else {
            bubble(("Lost cluster leader-> become undecided" + clusterId.str()).c_str());
            myState = NodeState::UNDECIDED;
            clusterId = Ipv4Address::UNSPECIFIED_ADDRESS;
            scheduleAt(simTime(), clusterStateEvent);
        }
    }
    else if (myState == NodeState::LEADER) {
        int myNeighborsNum = 0;
        std::map<Ipv4Address, ClusterAlgIpv4Route*> leadersIdToRoute;
        // find all neighbor leaders
        for (int i = 0, j = rt->getNumRoutes(); i < j; i++) {
            Ipv4Route *route = rt->getRoute(i);
            ClusterAlgIpv4Route *clusterRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);
            if (clusterRoute != nullptr) {
                if (clusterRoute->distance == 1) {
                    myNeighborsNum += 1;
                    if (clusterRoute->state == NodeState::LEADER) {
                        leadersIdToRoute[clusterRoute->getGateway()] = clusterRoute;
                    }
                }
            }
        }

        // update neighbor leaders
        for (auto it = counterOfSeenNeighborsLeaders.begin(); it != counterOfSeenNeighborsLeaders.end(); it++) {
            auto otherLedaerId = it->first;
            auto search = leadersIdToRoute.find(otherLedaerId);
            // if leader was seen also previously => update
            if (search != leadersIdToRoute.end()) {
                leadersIdToRoute.erase(otherLedaerId);
                it->second += 1;

                // if seen too many times => destroy cluster
                if (it->second > par("maxLeaderRepeats").intValue()) {
                    //check who should destroy -> my cluster or their
                    if (search->second->neighborsNum > myNeighborsNum
                            || (search->second->neighborsNum == myNeighborsNum && search->first < interface80211ptr->getIpv4Address())) {
                        myState = NodeState::UNDECIDED;
                        clusterId = Ipv4Address::UNSPECIFIED_ADDRESS;
                        bubble(("Destroying cluster -> become undecided" + clusterId.str()).c_str());

                        scheduleAt(simTime() + par("clusterUndecided").doubleValue(), clusterStateEvent);
                        counterOfSeenNeighborsLeaders.clear();
                        return;
                    }
                }
            }
            // leader is already disappeared
            else {
                //https://stackoverflow.com/questions/4600567/how-can-i-delete-elements-of-a-stdmap-with-an-iterator
                counterOfSeenNeighborsLeaders.erase(it);
            }
        }

        // add new leaders
        for (auto newLeader = leadersIdToRoute.begin(); newLeader != leadersIdToRoute.end(); newLeader++) {
            counterOfSeenNeighborsLeaders[newLeader->first] = 1;
        }

        scheduleAt(simTime() + par("clusterLeader").doubleValue(), clusterStateEvent);
    }
    else {
        throw cRuntimeError("Unknown node state");
    }
}

void ClusterAlg::handleMessageWhenUp(cMessage *msg)
{
    rt->purge(); // remove invalid ipv4route

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
        emit(helloSignal, 1);
        auto recHello = staticPtrCast<ClusterAlgHello>(check_and_cast<Packet*>(msg)->peekData<ClusterAlgHello>()->dupShared());
        receiveHello(recHello);
    }

    else if (type == MessageType::TOPOLOGY_CONTROL) {
        emit(topologyControlSignal, 1);
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

    Ipv4Address srcId = recHello->getSrcId();
    int numHops = recHello->getHopdistance(); // should be == 1

    Ipv4Route *route = rt->findBestMatchingRoute(srcId);
    ClusterAlgIpv4Route *clusterAlgRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);

    // if noRoute or newerSequenceNumber or otherNodeHasBetterMetric
    if (route == nullptr || clusterAlgRoute == nullptr
            || (clusterAlgRoute->getIdFromSource() == srcId && clusterAlgRoute->sequencenumber < sequencenumber)
            || (clusterAlgRoute->getIdFromSource() != srcId && clusterAlgRoute->getMetric() > numHops)) {
        removeOldRoute(clusterAlgRoute);
        addNewRoute(srcId, srcId, srcId, numHops, recHello);
    }

    numHops += 1;
    for (int i = 0, j = recHello->getNeighborsNum(); i < j; i++) {
        Ipv4Address neighbor = recHello->getNeighbors(i);

        route = rt->findBestMatchingRoute(neighbor);
        clusterAlgRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);

        if (route == nullptr || clusterAlgRoute == nullptr
                || (clusterAlgRoute->getIdFromSource() == srcId && clusterAlgRoute->sequencenumber < sequencenumber)
                || (clusterAlgRoute->getIdFromSource() != srcId && clusterAlgRoute->getMetric() > numHops)) {
            removeOldRoute(clusterAlgRoute);
            addNewRoute(neighbor, srcId, srcId, numHops, recHello);
        }
    }

}

inline void ClusterAlg::removeOldRoute(ClusterAlgIpv4Route *route)
{
    if (route != nullptr) {
        rt->deleteRoute(route);
    }
}

void ClusterAlg::addNewRoute(Ipv4Address dest, Ipv4Address next, Ipv4Address source, int distance, IntrusivePtr<inet::ClusterAlgHello> &recHello)
{
    ClusterAlgIpv4Route *e = new ClusterAlgIpv4Route();
    e->setDestination(dest);
    e->setGateway(next);
    e->setSourceFromId(source);
    e->setMetric(distance);
    e->distance = distance;
    e->sequencenumber = recHello->getSequencenumber();
    e->undecidedNeighborsNum = recHello->getUndecidedNeighborsNum();
    e->state = recHello->getState();
    e->neighborsNum = recHello->getNeighborsArraySize();
    e->setExpiryTime(simTime() + routeLifetime);

    e->setNetmask(Ipv4Address::ALLONES_ADDRESS);
    e->setInterface(interface80211ptr);
    e->setSourceType(IRoute::MANET);
    rt->addRoute(e);
}

void ClusterAlg::receiveTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{

}

} // namespace inet

