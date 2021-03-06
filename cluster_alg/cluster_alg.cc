#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
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
    delete additonalForwardEvent;
    clusterGraph->clear();
    delete clusterGraph;
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
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);

        routeLifetime = par("routeLifetime").doubleValue();
        helloInterval = par("helloInterval");
        tcInterval = par("tcInterval");
        debugAlert = par("debugAlert");
        useAdditionalForward = par("useAdditionalForward");

        helloEvent = new cMessage("helloEvent");
        tcEvent = new cMessage("topologyEvent");
        clusterStateEvent = new cMessage("clusterStateEvent");
        topolgyControlEvent = new cMessage("topolgyControlEvent");
        additonalForwardEvent = new cMessage("additonalForwardEvent");

        WATCH(myState);
        WATCH(clusterId);
        WATCH(myIp);
        WATCH(addressToClusterSize);
        WATCH(hasTarget);

        helloSignal = registerSignal("helloSignal");
        topologyControlSignal = registerSignal("topologyControlSignal");
        stateChangedSignal = registerSignal("stateChangedSignal");
        clusterDestroyedSignal = registerSignal("clusterDestroyedSignal");
        messageSendSignal = registerSignal("messageSendSignal");
        messageTransferedSignal = registerSignal("messageTransferedSignal");
        messageReceivedSignal = registerSignal("messageReceivedSignal");

    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        networkProtocol->registerHook(0, this);
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

    auto time = simTime()
            + par("clusterUndecidedFirstTime").doubleValue()
            + uniform(0.0, par("maxVariance").doubleValue());
    scheduleAt(time, clusterStateEvent);

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
    cancelEvent(additonalForwardEvent);
}

void ClusterAlg::handleSelfMessage(cMessage *msg)
{
    if (msg == helloEvent) {
        handleHelloEvent();
    }
    else if (msg == tcEvent) {
        handleTopologyEvent();
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
    else if (msg == additonalForwardEvent) {
        sendAdditionalForward();
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

void ClusterAlg::handleTopologyEvent()
{
    forcingTc = false;

    int index;
    auto tc = makeShared<ClusterAlgTopologyControl>();

    //set base informationshelloInterval
    tc->setChunkLength(b(128)); ///size of message in bits
    topologySeq += 2;
    tc->setSequencenumber(sequencenumber);
    tc->setHopdistance(1);
    tc->setMessageType(TOPOLOGY_CONTROL);
    tc->setSrcId(interface80211ptr->getIpv4Address());

    // find neighbor clusters and my members
    std::set<Ipv4Address> neighborClusters;
    std::vector<Ipv4Address> members;

    rt->purge();
    for (int k = 0, total = rt->getNumRoutes(); k < total; k++) {
        ClusterAlgIpv4Route *route = dynamic_cast<ClusterAlgIpv4Route*>(rt->getRoute(k));
        if (route != nullptr && route->distance == 1) {
            if (route->clusterId == myIp) {
                auto id = route->getIdFromSource();
                members.push_back(id);
                neighborClusters.insert(route->neighborsClusterIds.begin(), route->neighborsClusterIds.end());
            }
            else {
                neighborClusters.insert(route->clusterId);
            }
        }
    }

    myMembers.clear();
    myMembers.insert(members.begin(), members.end());

    // set neighbor clusters
    tc->setNeighborsClusterArraySize(neighborClusters.size());
    index = 0;
    for (auto const &neighborCluster : neighborClusters) {
        tc->setNeighborsCluster(index, neighborCluster);
        index += 1;
    }

    //set my members
    tc->setMembersArraySize(members.size());
    index = 0;
    for (auto const &member : members) {
        tc->setMembers(index, member);
        index += 1;
    }

    // select nodes allowed to forward this message
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
                neighborClusters.insert(
                        clusterRoute->neighborsClusterIds.begin(),
                        clusterRoute->neighborsClusterIds.end());
            }
        }
    }

    neighborClusters.erase(myIp);
    for (auto leader : leaders) {
        neighborClusters.erase(leader);
        clusterToNode.erase(leader);
        nodeToCluster.erase(leader);
    }

    std::set<Ipv4Address> allowedToForward;
    for (auto remainingCluster : neighborClusters) {

        // find node with access to remainingCluster
        ClusterAlgIpv4Route *bestCandidate = findBestCandidateToForward(remainingCluster, clusterToNode);
        if (bestCandidate == nullptr) {
            continue;
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
                    allowedToForward.insert(node);
                    break;
                }
            }
        }

        allowedToForward.insert(bestCandidate->getIdFromSource());
    }

    tc->setAllowedToForwardArraySize(allowedToForward.size());
    int index = 0;
    for (auto atf : allowedToForward) {
        tc->setAllowedToForward(index, atf);
        index += 1;
    }

}

ClusterAlgIpv4Route* ClusterAlg::findBestCandidateToForward(
        Ipv4Address clusterId,
        std::multimap<Ipv4Address, ClusterAlgIpv4Route*> clusterToNode)
{
    ClusterAlgIpv4Route *bestCandidate = nullptr;
    for (auto itr = clusterToNode.find(clusterId); itr != clusterToNode.end(); itr++) {
        Ipv4Address mapCluster = itr->first;
        if (mapCluster != clusterId) {
            continue;
        }

        ClusterAlgIpv4Route *candidate = itr->second;
        if (bestCandidate == nullptr) {
            bestCandidate = candidate;
        }
        else {
            if (candidate->getIdFromSource() < bestCandidate->getIdFromSource()) {
                bestCandidate = candidate;
            }
        }
    }
    return bestCandidate;
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
                neighborClusters.insert(
                        clusterRoute->neighborsClusterIds.begin(),
                        clusterRoute->neighborsClusterIds.end());
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
        std::vector<ClusterAlgIpv4Route*> leaderPropositions;

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
                    leaderPropositions.push_back(clusterRoute);
                }
                else if (clusterRoute->state == NodeState::UNDECIDED) {
                    myUndecidedDirectNeighbors += 1;

                    // if hasMoreUndecidedNeighbors or sameNumberButHigherId
                    if (clusterRoute->undecidedNeighborsNum > neighborBiggestUndecidedNeighbors
                            || (clusterRoute->undecidedNeighborsNum == neighborBiggestUndecidedNeighbors
                                    && clusterRoute->getGateway() > idUndecided)) {
                        neighborBiggestUndecidedNeighbors = clusterRoute->undecidedNeighborsNum;
                        idUndecided = clusterRoute->getGateway();
                    }
                }
            }
        }

        if (leaderPropositions.size() > 0) {
            auto bestLeaderIt = std::max_element(leaderPropositions.begin(), leaderPropositions.end(),
                    [](ClusterAlgIpv4Route const *a, ClusterAlgIpv4Route const *b) {
                        return a->acquaintanceCounter > b->acquaintanceCounter;
                    });

            if (bestLeaderIt == leaderPropositions.end()) {
                throw cRuntimeError("Cannot find leader");
            }
            ClusterAlgIpv4Route *bestLeader = *bestLeaderIt;

            myMembers.clear();
            myState = NodeState::MEMBER;
            clusterId = bestLeader->getGateway();
            scheduleAt(simTime() + par("clusterMember").doubleValue(), clusterStateEvent);
            bubble(("Become member of " + clusterId.str()).c_str());

            cancelEvent(helloEvent);
            scheduleAt(simTime(), helloEvent);
            return;
        }

        // become leader if number of undecided direct neighbors is greatest along all neighbors
        if (neighborBiggestUndecidedNeighbors < myUndecidedDirectNeighbors
                || (neighborBiggestUndecidedNeighbors == myUndecidedDirectNeighbors
                        && idUndecided > interface80211ptr->getIpv4Address())) {
            myMembers.clear();
            myState = NodeState::LEADER;
            clusterId = interface80211ptr->getIpv4Address();
            scheduleAt(simTime() + par("clusterLeader").doubleValue(), clusterStateEvent);
            bubble(("Become leader " + clusterId.str()).c_str());

            cancelEvent(helloEvent);
            scheduleAt(simTime(), helloEvent);
            scheduleTopologyControl(simTime()
                    + uniform(0.0, par("maxVariance").doubleValue())
                    + broadcastDelay->doubleValue());
            return;
        }

        scheduleAt(
                simTime() + par("clusterUndecided").doubleValue() + uniform(0.0, par("maxVariance").doubleValue()),
                clusterStateEvent);

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
                            || (search->second->neighborsNum == myNeighborsNum
                                    && search->first < interface80211ptr->getIpv4Address())) {
                        myMembers.clear();
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

void ClusterAlg::checkIp()
{
    if (debugAlert) {
        auto debugIpStr = par("debugIp").stringValue();
        if (debugIpStr == myIp.str(true)) {

            auto eventNumber = getSimulation()->getEventNumber();
            auto simTime = getSimulation()->getSimTime().str().c_str();
            char text[128];
            sprintf(text, "Event number: %ld, simTime: %s", eventNumber, simTime);

            auto otherIpStr = par("otherIp").stringValue();
            auto otherIp = Ipv4Address(otherIpStr);
            auto it = this->addressToCluster.find(otherIp);

            if (it != this->addressToCluster.end()) {
                if (!this->hasTarget) {
                    this->hasTarget = true;
                }
            }
            else {
                if (this->hasTarget) {
                    this->hasTarget = false;
                }
            }
        }
    }
}

void ClusterAlg::handleMessageWhenUp(cMessage *msg)
{
    checkIp();
    rt->purge(); // remove invalid ipv4route
    checkIp();

    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
        checkIp();
        return;
    }
    else if (check_and_cast<Packet*>(msg)->getTag<PacketProtocolTag>()->getProtocol() != &Protocol::manet) {
        throw cRuntimeError("Message not supported %s", msg->getName());
    }

    if (!msg->arrivedOn("ipIn")) {
        throw cRuntimeError("Message arrived on unknown gate %s", msg->getArrivalGate()->getName());
    }

    auto type = staticPtrCast<ClusterAlgBase>(
            check_and_cast<Packet*>(msg)->peekData<ClusterAlgBase>()->dupShared())->getMessageType();

    if (type == MessageType::HELLO) {
        emit(helloSignal, 1);
        auto hello = staticPtrCast<ClusterAlgHello>(
                check_and_cast<Packet*>(msg)->peekData<ClusterAlgHello>()->dupShared());
        receiveHello(hello);
    }

    else if (type == MessageType::TOPOLOGY_CONTROL) {
        emit(topologyControlSignal, 1);
        auto recTopologyControl = staticPtrCast<ClusterAlgTopologyControl>(
                check_and_cast<Packet*>(msg)->peekData<ClusterAlgTopologyControl>()->dupShared());
        receiveTopologyControl(recTopologyControl);
    }

    else {
        throw cRuntimeError("Unknown message type");
    }
    checkIp();
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
    if (route == nullptr || clusterAlgRoute == nullptr
            || (clusterAlgRoute->getIdFromSource() == srcId && clusterAlgRoute->sequencenumber < seq)
            || (clusterAlgRoute->getIdFromSource() != srcId && clusterAlgRoute->getMetric() > numHops)) {
        removeOldRoute(clusterAlgRoute);
        ClusterAlgIpv4Route *newRoute = addNewRoute(srcId, srcId, srcId, numHops, recHello);

        // leader route must have shorter life time
        if (srcId == clusterId) {
            newRoute->setExpiryTime(simTime() + helloInterval * 1.01);
        }
        if (newRoute->clusterId == myIp && myState == NodeState::LEADER) {
            auto memberIt = myMembers.find(srcId);
            if (memberIt != myMembers.end()) {
                // it is a known member
            }
            else {
                // it is a new one
                myMembers.insert(srcId);
                if (!forcingTc) {
                    forcingTc = true;
                    scheduleTopologyControl(simTime() + par("newMemberTopologyDelay"));
                }
            }
        }

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

        if (route == nullptr || clusterAlgRoute == nullptr
                || (clusterAlgRoute->getIdFromSource() == srcId && clusterAlgRoute->sequencenumber < seq)
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

ClusterAlgIpv4Route* ClusterAlg::addNewRoute(Ipv4Address dest, Ipv4Address next,
        Ipv4Address source, int distance,
        IntrusivePtr<inet::ClusterAlgHello> &recHello)
{
    ClusterAlgIpv4Route *e = new ClusterAlgIpv4Route();
    e->setDestination(dest);
    e->setGateway(next);
    e->setSourceFromId(source);
    e->setMetric(distance);
    e->distance = distance;
    e->acquaintanceCounter = (distance == 1) ? e->acquaintanceCounter + 1 : 0;
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
    bool isNewResult = updateTopologyControl(topologyControl);
    if (isNewResult) {
        recomputeRoute();
        std::string graphString = ClusterNode::toString(clusterGraph);
        EV_DEBUG << graphString;
    }

    if (canForwardTC(topologyControl, isNewResult)) {
        if (myState == NodeState::LEADER && isNewResult) {
            forwardTC(topologyControl, true);
        }
        else if (hasTopologyControlMyId(topologyControl)) {
            forwardTC(topologyControl, false);
        }
        else if (useAdditionalForward) {
            additionalForward(topologyControl);
        }
    }
}
void ClusterAlg::additionalForward(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{
    bool found = false;
    int counter = 0;
    for (auto val : additionalForwardQueue) {
        if (val.first->getSrcId() == topologyControl->getSrcId()
                && val.first->getSequencenumber() <= topologyControl->getSequencenumber()) {
            found = true;
            break;
        }
        counter += 1;
    }
    if (found) {
        additionalForwardQueue.erase(additionalForwardQueue.begin() + counter);
        if (!additionalForwardQueue.size()) {
            cancelEvent(additonalForwardEvent);
        }
        auto clusterInfo = getClusterInfoForTC(topologyControl);
        if (clusterInfo != nullptr) {
            clusterInfo->isAdditionalForwardExecuted = true;
        }
    }
    else {
        auto clusterInfo = getClusterInfoForTC(topologyControl);
        if (clusterInfo != nullptr && clusterInfo->isAdditionalForwardExecuted) {
            return;
        }

        SimTime time = simTime() + par("additionalForwardDelay").doubleValue() + this->uniform(0.001, 0.005);
        additionalForwardQueue.push_back(std::pair<IntrusivePtr<
                inet::ClusterAlgTopologyControl>, SimTime>(topologyControl, time));
        if (!additonalForwardEvent->isScheduled()) {
            scheduleAt(time, additonalForwardEvent);
        }
    }
}

void ClusterAlg::sendAdditionalForward()
{
    if (additionalForwardQueue.empty()) {
        return;
    }
    auto curTime = simTime();
    bool found = false;
    int index = 0;
    for (auto val : additionalForwardQueue) {
        if (val.second <= curTime) {
            found = true;
            break;
        }
        index += 1;
    }

    if (found) {
        auto toSend = additionalForwardQueue.at(index).first;
        additionalForwardQueue.erase(additionalForwardQueue.begin() + index);

        auto clusterInfo = getClusterInfoForTC(toSend);
        if (clusterInfo != nullptr) {
            // if not forwarded yet
            if (!clusterInfo->isAdditionalForwardExecuted) {
                clusterInfo->isAdditionalForwardExecuted = true;
                forwardTC(toSend, false, true);
            }
        }
        else {
            forwardTC(toSend, false, true);
        }
    }

    if (additionalForwardQueue.size()) {
        auto val = additionalForwardQueue.at(0);
        if (val.second <= simTime()) {
            sendAdditionalForward();
        }
        else {
            scheduleAt(val.second, additonalForwardEvent);
        }

    }

}

bool ClusterAlg::canForwardTC(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl, bool isNewResult)
{
    if (topologyControl->getSrcId() == myIp) {
        return false;
    }

    // ignore old topologyControl
    auto clusterInfo = getClusterInfoForTC(topologyControl);
    if (clusterInfo->seq != topologyControl->getSequencenumber()) {
        return false;
    }

    // ignore already forwarded
    if (clusterInfo->isForwarded) {
        return false;
    }

    // leaders always forward new messages
    if (myState == NodeState::LEADER) {
        if (isNewResult) {
            return true;
        }
        return false;
    }
    return true;
}
bool ClusterAlg::hasTopologyControlMyId(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{
    // otherwise forward only if my id is in allowed to forward list
    // we must check every time, because it is possible that our leader add our id to the list
    for (int i = 0, j = topologyControl->getAllowedToForwardArraySize(); i < j; i++) {
        Ipv4Address allowedToForward = topologyControl->getAllowedToForward(i);
        if (allowedToForward == myIp) {
            return true;
        }
    }
    return false;
}
ClusterInfo* ClusterAlg::getClusterInfoForTC(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{
    auto src = topologyControl->getSrcId();
    auto it = addressToCluster.find(src);
    if (it != addressToCluster.end()) {
        ClusterInfo *clusterInfo = it->second;
        return clusterInfo;
    }
    return nullptr;
}

void ClusterAlg::setForwardedTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{
    auto clusterInfo = getClusterInfoForTC(topologyControl);
    if (clusterInfo != nullptr) {
        clusterInfo->isForwarded = true;
    }
}

void ClusterAlg::recomputeRoute()
{
    rt->purge();
    clusterGraph->clear();
    idToNode.clear();

    // add cluster heads
    for (std::map<Ipv4Address, ClusterInfo*>::iterator it = addressToCluster.begin(); it != addressToCluster.end();) {
        Ipv4Address addr = it->first;
        ClusterInfo *cluster = it->second;

        if (addr == cluster->clusterId) {
            EV << addr.str();
            auto node = new ClusterNode(cluster);
            clusterGraph->addNode(node);
            idToNode.insert(std::pair<Ipv4Address, ClusterNode*>(addr, node));
        }
        ++it;
    }

    // add self
    //delete myNode;
    myNode = new ClusterNode(this->myIp);
    clusterGraph->addNode(myNode);
    idToNode.insert(std::pair<Ipv4Address, ClusterNode*>(this->myIp, myNode));

    //search for my members and neighbor clusters
    std::set<Ipv4Address> neighborClusters;
    std::map<Ipv4Address, ClusterAlgIpv4Route*> oneHopNeighborToRoute;

    for (int k = 0, total = rt->getNumRoutes(); k < total; k++) {
        ClusterAlgIpv4Route *route = dynamic_cast<ClusterAlgIpv4Route*>(rt->getRoute(k));
        if (route != nullptr && route->getMetric() == 1) {
            if (route->clusterId == myIp) {
                neighborClusters.insert(route->neighborsClusterIds.begin(), route->neighborsClusterIds.end());
            }
            else {
                neighborClusters.insert(route->clusterId);
            }

            auto id = route->getIdFromSource();
            oneHopNeighborToRoute.insert(std::pair<Ipv4Address, ClusterAlgIpv4Route*>(id, route));

        }
    }

    // add direct neighbors
    for (auto it = oneHopNeighborToRoute.begin(); it != oneHopNeighborToRoute.end(); it++) {
        Ipv4Address oneHopNeighbor = it->first;
        ClusterAlgIpv4Route *route = it->second;

        int weight = 1;
        auto link = new ClusterLink(weight);
        link->relation = "1hop";

        ClusterNode *node;
        auto nodeExist = idToNode.find(oneHopNeighbor);
        if (nodeExist == idToNode.end()) {
            // if node not exist yet then create and add the new node
            node = new ClusterNode(oneHopNeighbor);
            clusterGraph->addNode(node);
            idToNode.insert(std::pair<Ipv4Address, ClusterNode*>(oneHopNeighbor, node));

            // if node is not leader then add link to its leader
            if (route->clusterId != oneHopNeighbor) {
                auto nodeExist2 = idToNode.find(route->clusterId);
                if (nodeExist2 != idToNode.end()) {
                    auto leaderNode = nodeExist2->second;
                    int weight = 1;
                    auto link = new ClusterLink(weight);
                    link->relation = "1hop - its L";
                    clusterGraph->addLink(link, node, leaderNode);
                }
                else {
                    // we should know already leader node
                }
            }
            else {
                // if node is leader then use other relation
                int weight = 1;
                delete link;
                link = new ClusterLink(weight);
                link->relation = "1hop L";
            }

        }
        else {
            node = nodeExist->second;
        }

        clusterGraph->addLink(link, myNode, node);
    }

    // add 2hop neighbors
    std::map<Ipv4Address, ClusterNode*> twoHopNodesJustAdded;
    for (auto it = oneHopNeighborToRoute.begin(); it != oneHopNeighborToRoute.end(); it++) {
        Ipv4Address oneHopNeighbor = it->first;
        ClusterAlgIpv4Route *oneHopNeighborRoute = it->second;
        std::vector<Ipv4Address> twoHopneighborsVec = oneHopNeighborRoute->neighborsIds;
        std::vector<Ipv4Address> twoHopneighborsClusterVec = oneHopNeighborRoute->neighborsClusterIds;

        auto nodeExist = idToNode.find(oneHopNeighbor);
        if (nodeExist == idToNode.end()) {
            continue; // node should be already added, so this should never be happen
        }
        ClusterNode *oneHopNode = nodeExist->second;
        ClusterNode *twoHopNode;

        for (int i = 0, j = twoHopneighborsVec.size(); i < j; i++) {
            Ipv4Address twoHopNeighbor = twoHopneighborsVec.at(i);
            Ipv4Address twoHopNeighborCluster = twoHopneighborsClusterVec.at(i);

            auto twoHopNodeExist = idToNode.find(twoHopNeighbor);
            if (twoHopNodeExist != idToNode.end()) {
                // skip already added nodes
                continue;
            }

            auto twoHopNodeExist2 = twoHopNodesJustAdded.find(twoHopNeighbor);
            if (twoHopNodeExist2 == twoHopNodesJustAdded.end()) {
                // if node not exist yet then create and add the new node
                twoHopNode = new ClusterNode(twoHopNeighbor);
                clusterGraph->addNode(twoHopNode);
                twoHopNodesJustAdded.insert(std::pair<Ipv4Address, ClusterNode*>(twoHopNeighbor, twoHopNode));

                // find node of 2 hop neighbor cluster leader
                ClusterNode *twoHopNeighborClusterNode = nullptr;
                auto clusterNodeExist = idToNode.find(twoHopNeighborCluster);
                if (clusterNodeExist != idToNode.end()) {
                    twoHopNeighborClusterNode = clusterNodeExist->second;
                }
                else {
                    auto clusterNodeExist2 = twoHopNodesJustAdded.find(twoHopNeighborCluster);
                    if (clusterNodeExist2 != twoHopNodesJustAdded.end()) {
                        twoHopNeighborClusterNode = clusterNodeExist2->second;
                    }
                    else {
                        //cluster head should be added already
                    }
                }

                // connect 2 hop neighbor to its cluster
                if (twoHopNeighborClusterNode != nullptr && twoHopNeighborClusterNode != twoHopNode) {
                    int weight = 2;
                    auto link = new ClusterLink(weight);
                    link->relation = "2hop - its L";
                    clusterGraph->addLink(link, twoHopNode, twoHopNeighborClusterNode);
                }

            }
            else {
                // if node exist here it means that there are multiple path
                twoHopNode = twoHopNodeExist2->second;
            }

            int weight = 2;
            auto link = new ClusterLink(weight);
            link->relation = "1hop - 2hop";
            clusterGraph->addLink(link, oneHopNode, twoHopNode);
        }
    }

    //add connection between cluster heads
    for (int i = 0, j = clusterGraph->getNumNodes(); i < j; i++) {
        auto curNode = dynamic_cast<ClusterNode*>(clusterGraph->getNode(i));

        // it is this node
        if (curNode->clusterInfo == nullptr) {
            continue;
        }

        for (Ipv4Address otherClusterId : curNode->clusterInfo->neighborClusters) {
            auto otherNode = idToNode.find(otherClusterId);
            if (otherNode == idToNode.end()) {
                continue;
            }
            if (otherNode->second == curNode) {
                continue;
            }

            int weight = 8; // must be greater than direct and 2-hop neighbors
            auto link = new ClusterLink(weight);
            link->relation = "L-L";
            // graph is directed but we get reverse link when we process other node
            clusterGraph->addLink(link, curNode, otherNode->second);
        }
    }

    addressToClusterSize = addressToCluster.size();
}

bool ClusterAlg::updateTopologyControl(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl)
{
    auto src = topologyControl->getSrcId();

    if (src == myIp) {
        return false;
    }

    //remove expired info
    auto curTime = simTime();

    // collect cluster info to delete
    std::set<ClusterInfo*> toDelete;
    for (auto it = addressToCluster.begin(); it != addressToCluster.end(); it++) {
        ClusterInfo *clusterInfo = it->second;
        if (clusterInfo->expiryTime < curTime) {
            toDelete.insert(clusterInfo);
        }
    }
    // delete cluster info
    for (ClusterInfo *clusterToDelete : toDelete) {
        for (Ipv4Address member : clusterToDelete->members) {
            auto memberExist = addressToCluster.find(member);
            if (memberExist != addressToCluster.end()) {
                auto currentCluster = memberExist->second;
                if (currentCluster->clusterId == clusterToDelete->clusterId) {
                    addressToCluster.erase(member);
                }
            }
        }

        addressToCluster.erase(clusterToDelete->clusterId);
        delete clusterToDelete;
    }

    //check if info exists
    auto it = addressToCluster.find(src);
    if (it != addressToCluster.end()) {
        auto clusterToDelete = it->second;

        // check if sequence is newer
        if (clusterToDelete->seq >= topologyControl->getSequencenumber()) {
            return false;
        }

        // first delete members
        for (Ipv4Address member : clusterToDelete->members) {
            auto memberExist = addressToCluster.find(member);
            if (memberExist != addressToCluster.end()) {
                auto currentCluster = memberExist->second;
                if (currentCluster->clusterId == clusterToDelete->clusterId) {
                    addressToCluster.erase(member);
                }
            }
        }

        // delete cluster info
        addressToCluster.erase(clusterToDelete->clusterId);
        delete clusterToDelete;

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
    forwardTC(topologyControl, resetForwardNodes, false);
}

void ClusterAlg::forwardTC(IntrusivePtr<inet::ClusterAlgTopologyControl> &topologyControl,
        bool resetForwardNodes, bool additionalForward)
{
    setForwardedTopologyControl(topologyControl);

    auto tc = makeShared<ClusterAlgTopologyControl>();

    tc->setChunkLength(b(128)); ///size of message in bits
    tc->setSequencenumber(topologyControl->getSequencenumber());
    tc->setHopdistance(topologyControl->getHopdistance() + 1);
    tc->setMessageType(TOPOLOGY_CONTROL);
    tc->setSrcId(topologyControl->getSrcId());

    if (resetForwardNodes) {
        setAllowedToForwardNodes(tc);
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

    int membersSize = topologyControl->getMembersArraySize();
    tc->setMembersArraySize(membersSize);
    for (int i = 0; i < membersSize; i++) {
        tc->setMembers(i, topologyControl->getMembers(i));
    }

    //new control info for ClusterAlgTopologyControl
    auto packetName = resetForwardNodes ? "TopologyControlForwardFromLeader" : "TopologyControlForward";
    if (additionalForward) {
        packetName = "TopologyControlForwardAdditional";
    }
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

INetfilter::IHook::Result ClusterAlg::ensureRouteForDatagram(Packet *datagram)
{
    const auto &networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address &destAddr = networkHeader->getDestinationAddress();

    if (destAddr.getType() != L3Address::AddressType::IPv4) {
        throw cRuntimeError("Unsupported address type");
    }
    Ipv4Address address = destAddr.toIpv4();
    if (address == Ipv4Address::ALLONES_ADDRESS || address == Ipv4Address::LOOPBACK_ADDRESS) {
        return INetfilter::IHook::Result::ACCEPT;
    }

    Ipv4Address gateway = calculateRoute(address);
    if (debugAlert && gateway == Ipv4Address::UNSPECIFIED_ADDRESS) {
        debugAlert = false;
        auto eventNumber = getSimulation()->getEventNumber();
        auto simTime = getSimulation()->getSimTime().str().c_str();
        char text[128];
        sprintf(text, "Event number: %ld, simTime: %s, myIp %s searchFor %s",
                eventNumber, simTime,
                myIp.str(true).c_str(), address.str(true).c_str());
        getSimulation()->getActiveEnvir()->alert(text);

        auto gatewayStr2 = gateway.str(true);
        Ipv4Address gateway = calculateRoute(address);
    }

    auto gatewayStr = gateway.str(true);
    std::string graphString = ClusterNode::toString(clusterGraph);
    EV_DEBUG << graphString;

    return INetfilter::IHook::Result::ACCEPT;
}

Ipv4Address ClusterAlg::calculateRoute(Ipv4Address address)
{
    // route already exists
    Ipv4Route *bestMatch = rt->findBestMatchingRoute(address);
    if (bestMatch != nullptr) {
        return bestMatch->getGateway();
    }

    Ipv4Address nextAddress = Ipv4Address::UNSPECIFIED_ADDRESS;

    // try 2 hop neighbor
    if (nextAddress == Ipv4Address::UNSPECIFIED_ADDRESS) {
        for (int k = 0, total = rt->getNumRoutes(); k < total; k++) {
            ClusterAlgIpv4Route *route = dynamic_cast<ClusterAlgIpv4Route*>(rt->getRoute(k));

            if (route != nullptr && route->getMetric() == 1) {
                for (Ipv4Address twoHopNeighbor : route->neighborsIds) {
                    if (twoHopNeighbor == address) {
                        nextAddress = route->getIdFromSource();
                        addNewRoute(address, nextAddress, route->clusterId, 2, NodeState::MEMBER);
                        break;
                    }
                }

                if (nextAddress != Ipv4Address::UNSPECIFIED_ADDRESS) {
                    break;
                }
            }
        }
    }

    if (nextAddress == Ipv4Address::UNSPECIFIED_ADDRESS) {

        // route does not exist but the cluster is known
        auto addressToClusterIt = addressToCluster.find(address);
        if (addressToClusterIt != addressToCluster.end()) {
            ClusterInfo *clusterInfo = addressToClusterIt->second;
            Ipv4Address clusterLeaderIp = clusterInfo->clusterId;

            auto nodeIt = idToNode.find(clusterLeaderIp);
            if (nodeIt != idToNode.end()) {
                auto node = nodeIt->second;
                clusterGraph->calculateWeightedSingleShortestPathsTo(node);

                cTopology::LinkOut *outPath = myNode->getPath(0);
                if (outPath != nullptr) {
                    ClusterNode *remoteNode = dynamic_cast<ClusterNode*>(outPath->getRemoteNode());
                    if (remoteNode != nullptr) {
                        int distance = (int) myNode->getDistanceToTarget();
                        if (remoteNode->clusterInfo != nullptr) { //via neighbor cluster
                            nextAddress = remoteNode->clusterInfo->clusterId;
                            addNewRouteToClusterLeader(address, nextAddress, (int) distance);
                        }

                        else if (remoteNode->clusterRoute != nullptr) { //via neighbor
                            nextAddress = remoteNode->clusterRoute->getIdFromSource();
                            Ipv4Address clusterAddress = remoteNode->clusterRoute->clusterId;
                            addNewRoute(address, nextAddress, clusterAddress, distance, NodeState::MEMBER);
                        }

                        else if (remoteNode->address != Ipv4Address::UNSPECIFIED_ADDRESS) { //2 hop neighbor or self - probably never accessible
                            nextAddress = remoteNode->address;
                            addNewRoute(address, nextAddress, nextAddress, distance, NodeState::MEMBER);
                        }

                        else {
                            throw cRuntimeError("Unknown ClusterNode type");
                        }
                    }

                }

            }
        }
    }

    return nextAddress;

}

ClusterAlgIpv4Route* ClusterAlg::addNewRouteToClusterLeader(Ipv4Address dest, Ipv4Address clusterId, int distance)
{
    Ipv4Address gateway = Ipv4Address::UNSPECIFIED_ADDRESS;
    NodeState state = NodeState::LEADER;
    std::multimap<Ipv4Address, ClusterAlgIpv4Route*> clusterToNode;

    for (int i = 0, j = rt->getNumRoutes(); i < j; i++) {
        Ipv4Route *route = rt->getRoute(i);
        ClusterAlgIpv4Route *clusterRoute = dynamic_cast<ClusterAlgIpv4Route*>(route);
        if (clusterRoute == nullptr) {
            continue;
        }

        // if isDirectNeighbor
        if (clusterRoute->distance == 1) {
            if (clusterRoute->getIdFromSource() == clusterId) {
                gateway = clusterId;
                break;
            }
            if (clusterRoute->state == NodeState::MEMBER) {
                for (auto cluster : clusterRoute->neighborsClusterIds) {
                    clusterToNode.insert(std::pair<Ipv4Address, ClusterAlgIpv4Route*>(cluster, clusterRoute));
                }
            }
        }
    }

    if (gateway == Ipv4Address::UNSPECIFIED_ADDRESS) {
        auto bestCandidateToForward = this->findBestCandidateToForward(clusterId, clusterToNode);
        if (bestCandidateToForward != nullptr) {
            gateway = bestCandidateToForward->getIdFromSource();
            state = NodeState::MEMBER;
        }
        else {
            gateway = clusterId;
        }
    }

    return addNewRoute(dest, gateway, clusterId, distance, state);
}

ClusterAlgIpv4Route* ClusterAlg::addNewRoute(Ipv4Address dest, Ipv4Address gateway, Ipv4Address clusterId, int distance,
        NodeState state)
{
    Ipv4Address source = this->myIp;

    ClusterAlgIpv4Route *e = new ClusterAlgIpv4Route();
    e->setDestination(dest);
    e->setGateway(gateway);
    e->setSourceFromId(source);
    e->setMetric(distance + 1);
    e->distance = distance + 1; // only neighbors are allowed to have 1 distance

    e->clusterId = clusterId;
    e->sequencenumber = this->sequencenumber;
    this->sequencenumber += 1;

    e->undecidedNeighborsNum = 0;
    e->state = state;
    e->neighborsNum = 0;

    e->setExpiryTime(simTime() + routeLifetime);
    e->setNetmask(Ipv4Address::ALLONES_ADDRESS);
    e->setInterface(interface80211ptr);
    e->setSourceType(IRoute::MANET);
    rt->addRoute(e);
    return e;
}

} // namespace inet

