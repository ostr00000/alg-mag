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
    delete tcEvent;
    delete clusterStateEvent;
    delete topolgyControlEvent;
    clusterGraph->clear();
    delete clusterGraph;
    delete communicationMessageEvent;
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
        clusterGraph = new cTopology("clusterGraph");

        routeLifetime = par("routeLifetime").doubleValue();
        helloInterval = par("helloInterval");
        tcInterval = par("tcInterval");

        helloEvent = new cMessage("helloEvent");
        tcEvent = new cMessage("topologyEvent");
        clusterStateEvent = new cMessage("clusterStateEvent");
        topolgyControlEvent = new cMessage("topolgyControlEvent");
        communicationMessageEvent = new cMessage("communicationMessageEvent");

        WATCH(myState);
        WATCH(clusterId);
        WATCH(myIp);

        helloSignal = registerSignal("helloSignal");
        topologyControlSignal = registerSignal("topologyControlSignal");
        stateChangedSignal = registerSignal("stateChangedSignal");
        clusterDestroyedSignal = registerSignal("clusterDestroyedSignal");
        messageSendSignal = registerSignal("messageSendSignal");
        messageTransferedSignal = registerSignal("messageTransferedSignal");
        messageReceivedSignal = registerSignal("messageReceivedSignal");

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

    auto time = simTime() + par("clusterUndecidedFirstTime").doubleValue() + uniform(0.0, par("maxVariance").doubleValue());
    scheduleAt(time, clusterStateEvent);

    time = simTime() + uniform(0.0, par("messageSendVariance")) + par("messageSendTime");
    scheduleAt(time, communicationMessageEvent);

}

void ClusterAlg::scheduleTopologyControl(simtime_t scheduleTime)
{
    cancelEvent(tcEvent);
    scheduleAt(scheduleTime, tcEvent);
}

void ClusterAlg::stop()
{
    cancelEvent(tcEvent);
    cancelEvent(helloEvent);
    cancelEvent(clusterStateEvent);
}

void ClusterAlg::handleSelfMessage(cMessage *msg)
{
    if (msg == helloEvent) {
        handleHelloEvent();
    }
    else if (msg == tcEvent) {
        handleTopolgyEvent();
    }
    else if (msg == clusterStateEvent) {
        NodeState stateBefore = myState;
        handleClusterStateEvent();

        if (stateBefore != myState) {
            refreshTextFromState();
            emit(stateChangedSignal, 1);
            if (stateBefore == NodeState::LEADER) {
                emit(clusterDestroyedSignal, 1);
            }
        }
    }
    else if (msg) {
        handleCommunicationMessageEvent();
    }
}

void ClusterAlg::handleCommunicationMessageEvent()
{

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

    int offsetFromEnd = 4;
    std::string name = clusterId.str(true);
    auto cname = name.substr(name.size() - offsetFromEnd).c_str();
    if (myState == NodeState::UNDECIDED) {
        sprintf(label, "Undecided");
    }
    else if (myState == NodeState::LEADER) {
        sprintf(label, "[%s]-Leader", cname);
    }
    else if (myState == NodeState::MEMBER) {
        name = myIp.str(true);
        auto myNameC = name.substr(name.size() - offsetFromEnd).c_str();
        sprintf(label, "[%s]-Member of [%s]", myNameC, cname);
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
    std::list<Ipv4Address> neighborClusters;
    for (int k = 0, total = rt->getNumRoutes(); k < total; k++) {
        ClusterAlgIpv4Route *route = dynamic_cast<ClusterAlgIpv4Route*>(rt->getRoute(k));
        if (route != nullptr && route->getMetric() == 1) {
            oneHopNeighbors.push_back(route->getDestination());
            neighborClusters.push_back(route->clusterId);
        }
    }
    hello->setNeighborsArraySize(oneHopNeighbors.size());
    hello->setNeighborsClusterArraySize(oneHopNeighbors.size());

    int index = 0;
    for (auto const &n : oneHopNeighbors) {
        hello->setNeighbors(index, n);
        index += 1;
    }
    index = 0;
    for (auto const &n : neighborClusters) {
        hello->setNeighborsCluster(index, n);
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

void ClusterAlg::handleTopolgyEvent()
{
    auto tc = makeShared<ClusterAlgTopologyControl>();

    tc->setChunkLength(b(128)); ///size of message in bits
    topologySeq += 2;
    tc->setSequencenumber(sequencenumber);
    tc->setHopdistance(1);
    tc->setMessageType(TOPOLOGY_CONTROL);
    tc->setSrcId(interface80211ptr->getIpv4Address());

    setAllowedToForwardNodes(tc);

    //new control info for ClusterAlgTopologyControl
    auto packet = new Packet("TopologyControl", tc);
    auto addressReq = packet->addTag<L3AddressReq>();
    addressReq->setDestAddress(Ipv4Address(255, 255, 255, 255));
    addressReq->setSrcAddress(interface80211ptr->getIpv4Address());
    packet->addTag<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

    send(packet, "ipOut");

    scheduleTopologyControl(simTime() + tcInterval + broadcastDelay->doubleValue());
    bubble("Sending topology control message");
}

void ClusterAlg::setAllowedToForwardNodes(IntrusivePtr<inet::ClusterAlgTopologyControl> &tc)
{
    std::set<Ipv4Address> leaders;
    std::set<Ipv4Address> neighborClusters;
    std::multimap<Ipv4Address, ClusterAlgIpv4Route*> clusterToNode;
    std::multimap<Ipv4Address, Ipv4Address> nodeToCluster;

    // find neighbor leaders and all neighbor clusters
    for (int i = 0, j = rt->getNumRoutes(); i < j; i++) {
        Ipv4Route *route = rt->getRoute(i);
        ClusterAlgIpv4Route *clusterRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);
        if (clusterRoute == nullptr) {
            continue;
        }
        // if isDirectNeighbor
        if (clusterRoute->distance == 1) {
            if (clusterRoute->state == NodeState::LEADER) {
                leaders.insert(clusterRoute->clusterId);
            }
            else if (clusterRoute->state == NodeState::MEMBER) {
                auto id = clusterRoute->getIdFromSource();
                for (auto cluster : clusterRoute->neighborsClusterIds) {
                    clusterToNode.insert(std::pair<Ipv4Address, ClusterAlgIpv4Route*>(cluster, clusterRoute));
                    nodeToCluster.insert(std::pair<Ipv4Address, Ipv4Address>(id, cluster));
                }
                neighborClusters.insert(clusterRoute->neighborsClusterIds.begin(), clusterRoute->neighborsClusterIds.end());
            }
        }
    }

    neighborClusters.erase(myIp);
    for (auto leader : leaders) {
        neighborClusters.erase(leader);
        clusterToNode.erase(leader);
        nodeToCluster.erase(leader);
    }

    std::vector<Ipv4Address> allowedToForward;
    ClusterAlgIpv4Route *bestCandidate;
    bool first;
    for (auto remainingCluster : neighborClusters) {

        // find node with access to remainingCluster
        first = false;
        for (auto itr = clusterToNode.find(remainingCluster); itr != clusterToNode.end(); itr++) {
            Ipv4Address mapCluster = itr->first;
            if (mapCluster != remainingCluster) {
                continue;
            }

            ClusterAlgIpv4Route *candidate = itr->second;
            if (!first) {
                bestCandidate = candidate;
                first = true;
            }
            else {
                if (candidate->getIdFromSource() < bestCandidate->getIdFromSource()) {
                    bestCandidate = candidate;
                }
            }
        }

        // check if bestCandidate is direct neighbor with leader of remainingCluster
        bool needToFoundAnotherNode = true;
        for (Ipv4Address neighborId : bestCandidate->neighborsIds) {
            if (neighborId == remainingCluster) {
                needToFoundAnotherNode = false;
                break;
            }
        }
        // otherwise need to found second node to forward
        if (needToFoundAnotherNode) {
            for (int i = 0, j = bestCandidate->neighborsClusterIds.size(); i < j; i++) {
                auto c = bestCandidate->neighborsClusterIds[i];
                if (c == remainingCluster) {
                    auto node = bestCandidate->neighborsIds[i];
                    allowedToForward.push_back(node);
                    break;
                }
            }
        }

        allowedToForward.push_back(bestCandidate->getIdFromSource());
    }

    tc->setAllowedToForwardArraySize(allowedToForward.size());
    int index = 0;
    for (auto atf : allowedToForward) {
        tc->setAllowedToForward(index, atf);
    }

}

void ClusterAlg::setNeighborsCluster(IntrusivePtr<inet::ClusterAlgTopologyControl> &tc)
{
    std::set<Ipv4Address> neighborClusters;

    // find neighbor leaders and all neighbor clusters
    for (int i = 0, j = rt->getNumRoutes(); i < j; i++) {
        Ipv4Route *route = rt->getRoute(i);
        ClusterAlgIpv4Route *clusterRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);
        if (clusterRoute == nullptr) {
            continue;
        }
        // if isDirectNeighbor
        if (clusterRoute->distance == 1) {
            if (clusterRoute->state == NodeState::LEADER) {
                neighborClusters.insert(clusterRoute->clusterId);
            }
            else if (clusterRoute->state == NodeState::MEMBER) {
                neighborClusters.insert(clusterRoute->neighborsClusterIds.begin(), clusterRoute->neighborsClusterIds.end());
            }
        }
    }
    neighborClusters.erase(myIp);

    tc->setNeighborsClusterArraySize(neighborClusters.size());
    int i = 0;
    for (Ipv4Address neighborCluster : neighborClusters) {
        tc->setNeighborsCluster(i, neighborCluster);
        i += 1;
    }

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
            scheduleAt(simTime(), helloEvent);
            scheduleTopologyControl(simTime() + tcInterval + uniform(0.0, par("maxVariance").doubleValue()) + broadcastDelay->doubleValue());
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
            cancelEvent(tcEvent);
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
        auto hello = staticPtrCast<ClusterAlgHello>(check_and_cast<Packet*>(msg)->peekData<ClusterAlgHello>()->dupShared());
        receiveHello(hello);
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
    unsigned int seq = recHello->getSequencenumber();

    Ipv4Route *route = rt->findBestMatchingRoute(srcId);
    ClusterAlgIpv4Route *clusterAlgRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);

    // if noRoute or newerSequenceNumber or otherNodeHasBetterMetric
    if (route == nullptr || clusterAlgRoute == nullptr || (clusterAlgRoute->getIdFromSource() == srcId && clusterAlgRoute->sequencenumber < seq)
            || (clusterAlgRoute->getIdFromSource() != srcId && clusterAlgRoute->getMetric() > numHops)) {
        removeOldRoute(clusterAlgRoute);
        ClusterAlgIpv4Route *newRoute = addNewRoute(srcId, srcId, srcId, numHops, recHello);

        for (int i = 0, c = recHello->getNeighborsClusterArraySize(); i < c; i++) {
            newRoute->neighborsClusterIds.push_back(recHello->getNeighborsCluster(i));
        }
        for (int i = 0, c = recHello->getNeighborsArraySize(); i < c; i++) {
            newRoute->neighborsIds.push_back(recHello->getNeighbors(i));
        }
    }

    numHops += 1;
    for (int i = 0, j = recHello->getNeighborsNum(); i < j; i++) {
        Ipv4Address neighbor = recHello->getNeighbors(i);

        route = rt->findBestMatchingRoute(neighbor);
        clusterAlgRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);

        if (route == nullptr || clusterAlgRoute == nullptr || (clusterAlgRoute->getIdFromSource() == srcId && clusterAlgRoute->sequencenumber < seq)
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

ClusterAlgIpv4Route* ClusterAlg::addNewRoute(Ipv4Address dest, Ipv4Address next, Ipv4Address source, int distance,
        IntrusivePtr<inet::ClusterAlgHello> &recHello)
{
    ClusterAlgIpv4Route *e = new ClusterAlgIpv4Route();
    e->setDestination(dest);
    e->setGateway(next);
    e->setSourceFromId(source);
    e->setMetric(distance);
    e->distance = distance;
    e->clusterId = recHello->getClusterHeadId();
    e->sequencenumber = recHello->getSequencenumber();
    e->undecidedNeighborsNum = recHello->getUndecidedNeighborsNum();
    e->state = recHello->getState();
    e->neighborsNum = recHello->getNeighborsArraySize();
    e->setExpiryTime(simTime() + routeLifetime);

    e->setNetmask(Ipv4Address::ALLONES_ADDRESS);
    e->setInterface(interface80211ptr);
    e->setSourceType(IRoute::MANET);
    rt->addRoute(e);
    return e;
}
void ClusterAlg::receiveTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{
    bool newResut = updateTopologyControl(topologyControl);
    if (!newResut) {
        return;
    }
    recomputeRoute();

    if (myState == NodeState::LEADER) {
        forwardTC(topologyControl, true);
    }
    else if (myState == NodeState::MEMBER) {
        //check if should forward
        bool shouldForward = false;
        for (int i = 0, j = topologyControl->getAllowedToForwardArraySize(); i < j; i++) {
            Ipv4Address allowedToForward = topologyControl->getAllowedToForward(i);
            if (allowedToForward == myIp) {
                shouldForward = true;
                break;
            }
        }

        if (shouldForward) {
            forwardTC(topologyControl, false);
        }
    }
}

void ClusterAlg::recomputeRoute()
{
    clusterGraph->clear();
    std::map<Ipv4Address, ClusterNode*> clusterIdToNode;

    // add cluster heads
    std::map<Ipv4Address, ClusterInfo*>::iterator it = addressToCluster.begin();
    while (it != addressToCluster.end()) {
        Ipv4Address addr = it->first;
        ClusterInfo *cluster = it->second;

        if (addr == cluster->clusterId) {
            auto node = new ClusterNode(cluster);
            clusterGraph->addNode(node);
            clusterIdToNode.insert(std::pair<Ipv4Address, ClusterNode*>(addr, node));
        }
        ++it;
    }

    //add connection between cluster heads
    for (int i = 0, j = clusterGraph->getNumNodes(); i < j; i++) {
        auto curNode = dynamic_cast<ClusterNode*>(clusterGraph->getNode(i));

        for (Ipv4Address otherClusterId : curNode->clusterInfo->neighborClusters) {
            auto otherNode = clusterIdToNode.find(otherClusterId);
            if (otherNode == clusterIdToNode.end()) {
                continue;
            }

            int weight = 4; // must be greater than direct and 2-hop neighbors
            auto link = new cTopology::Link(weight);
            // graph is directed but we get reverse link when we process other node
            clusterGraph->addLink(link, curNode, otherNode->second);
        }
    }

    // add self
    auto myNode = new ClusterNode((ClusterAlgIpv4Route*) nullptr);
    clusterGraph->addNode(myNode);

    // add direct neighbors
    for (int i = 0, j = rt->getNumRoutes(); i < j; i++) {
        Ipv4Route *route = rt->getRoute(i);
        ClusterAlgIpv4Route *clusterRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);
        if (clusterRoute == nullptr) {
            continue;
        }

        ClusterNode *node;
        auto nodeExist = clusterIdToNode.find(clusterRoute->clusterId);
        if (nodeExist == clusterIdToNode.end()) {
            // if node not exist yet then create and add the new node
            node = new ClusterNode(clusterRoute);
            clusterGraph->addNode(node);
        }
        else {
            node = nodeExist->second;
        }

        int weight = 1;
        auto link = new cTopology::Link(weight);
        clusterGraph->addLink(link, myNode, node);
    }

}

bool ClusterAlg::updateTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{
    auto src = topologyControl->getSrcId();

    if (src == myIp) {
        return false;
    }

    //remove expired info
    auto curTime = simTime();

    std::map<Ipv4Address, ClusterInfo*>::iterator it = addressToCluster.begin();
    while (it != addressToCluster.end()) {
        ClusterInfo *clusterInfo = it->second;

        // need to delete
        if (clusterInfo->expiryTime < curTime) {

            // first delete members
            for (Ipv4Address member : clusterInfo->members) {

                // delete if member exist in map
                auto clusterIt = addressToCluster.find(member);
                if (clusterIt != addressToCluster.end()) {

                    // delete member only if it still belongs to this cluster
                    if (clusterIt->second->clusterId == clusterInfo->clusterId) {
                        addressToCluster.erase(member);
                    }

                }
            }

            // delete cluster info
            delete clusterInfo;
            it = addressToCluster.erase(it);

        }
        else {
            ++it;
        }
    }

    //check if info exists
    it = addressToCluster.find(src);
    if (it != addressToCluster.end()) {
        auto clusterInfo = it->second;

        // check if sequence is newer
        if (clusterInfo->seq >= topologyControl->getSequencenumber()) {
            return false;
        }

        // first delete members
        for (Ipv4Address member : clusterInfo->members) {

            // delete if member exist in map
            auto memberClusterIt = addressToCluster.find(member);
            if (memberClusterIt != addressToCluster.end()) {

                // delete member only if it still belongs to this cluster
                if (memberClusterIt->second->clusterId == clusterInfo->clusterId) {
                    addressToCluster.erase(member);
                }
            }
        }

        // delete cluster info
        delete clusterInfo;
        it = addressToCluster.erase(it);

    }

    //add new info
    ClusterInfo *ci = new ClusterInfo();
    ci->seq = topologyControl->getSequencenumber();
    ci->clusterId = src;
    ci->expiryTime = simTime() + routeLifetime;

    for (int i = 0, j = topologyControl->getNeighborsClusterArraySize(); i < j; i++) {
        auto neighbor = topologyControl->getNeighborsCluster(i);
        ci->neighborClusters.push_back(neighbor);
    }

    for (int i = 0, j = topologyControl->getMembersArraySize(); i < j; i++) {
        auto member = topologyControl->getMembers(i);
        ci->members.push_back(member);
        addressToCluster.insert(std::pair<Ipv4Address, ClusterInfo*>(member, ci));
    }

    addressToCluster.insert(std::pair<Ipv4Address, ClusterInfo*>(src, ci));
    return true;

}

void ClusterAlg::forwardTC(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl, bool resetForwardNodes)
{
    auto tc = makeShared<ClusterAlgTopologyControl>();

    tc->setChunkLength(b(128)); ///size of message in bits
    tc->setSequencenumber(topologyControl->getSequencenumber());
    tc->setHopdistance(topologyControl->getHopdistance() + 1);
    tc->setMessageType(TOPOLOGY_CONTROL);
    tc->setSrcId(topologyControl->getSrcId());

    if (resetForwardNodes) {
        setAllowedToForwardNodes(topologyControl);
    }
    else {
        int forwadSize = topologyControl->getAllowedToForwardArraySize();
        tc->setAllowedToForwardArraySize(forwadSize);
        for (int i = 0; i < forwadSize; i++) {
            tc->setAllowedToForward(i, topologyControl->getAllowedToForward(i));
        }
    }

    int neighborClusterSize = topologyControl->getNeighborsClusterArraySize();
    tc->setNeighborsClusterArraySize(neighborClusterSize);
    for (int i = 0; i < neighborClusterSize; i++) {
        tc->setNeighborsCluster(i, topologyControl->getNeighborsCluster(i));
    }

    //new control info for ClusterAlgTopologyControl
    auto packetName = resetForwardNodes ? "TopologyControlForwardFromLeader" : "TopologyControlForward";
    auto packet = new Packet(packetName, tc);
    auto addressReq = packet->addTag<L3AddressReq>();
    addressReq->setDestAddress(Ipv4Address(255, 255, 255, 255));
    addressReq->setSrcAddress(interface80211ptr->getIpv4Address());
    packet->addTag<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

    sendDelayed(packet, par("broadcastDelay").doubleValue(), "ipOut");
    bubble("Forwarding topology control message");
}

} // namespace inet

