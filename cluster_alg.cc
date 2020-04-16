#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/routing/cluster_alg/cluster_alg.h"

namespace inet {

Define_Module(ClusterAlg);

ClusterAlg::ForwardEntry::~ForwardEntry()
{
    if (this->event != nullptr)
        delete this->event;
    if (this->hello != nullptr)
        delete this->hello;
}

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
}

void ClusterAlg::stop()
{
    cancelEvent(event);
}

void ClusterAlg::handleSelfMessage(cMessage *msg)
{
    if (msg == event) {
        auto hello = makeShared<ClusterAlgHello>();
        rt->purge(); // TODO

        Ipv4Address source = interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
        hello->setChunkLength(b(128)); ///size of Hello message in bits
        hello->setSrcAddress(source);
        sequencenumber += 2;
        hello->setSequencenumber(sequencenumber);
        hello->setNextAddress(source);
        hello->setHopdistance(1);

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

        //schedule new brodcast hello message event
        scheduleAt(simTime() + helloInterval + broadcastDelay->doubleValue(), event);
        bubble("Sending new hello message");
    }
}

void ClusterAlg::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    }
    else if (check_and_cast<Packet*>(msg)->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::manet) {
        auto packet = new Packet("Hello");

        // When ClusterAlg module receives ClusterAlgHello from other host
        // it adds/replaces the information in routing table for the one contained in the message
        // but only if it's useful/up-to-date. If not the ClusterAlg module ignores the message.
        auto addressReq = packet->addTag<L3AddressReq>();
        addressReq->setDestAddress(Ipv4Address(255, 255, 255, 255));
        addressReq->setSrcAddress(interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        packet->addTag<InterfaceReq>()->setInterfaceId(interface80211ptr->getInterfaceId());
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        auto recHello = staticPtrCast<ClusterAlgHello>(check_and_cast<Packet*>(msg)->peekData<ClusterAlgHello>()->dupShared());

        if (msg->arrivedOn("ipIn")) {
            bubble("Received hello message");
            Ipv4Address source = interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress();

            Ipv4Address src = recHello->getSrcAddress();
            unsigned int msgsequencenumber = recHello->getSequencenumber();
            int numHops = recHello->getHopdistance();
            Ipv4Address next = recHello->getNextAddress();

            if (src == source) {
                EV_INFO << "Hello msg dropped. This message returned to the original creator.\n";
                delete packet;
                delete msg;
                return;
            }

            Ipv4Route *_entrada_routing = rt->findBestMatchingRoute(src);
            ClusterAlgIpv4Route *entrada_routing = dynamic_cast<ClusterAlgIpv4Route*>(_entrada_routing);

            //Tests if the ClusterAlg hello message that arrived is useful
            if (_entrada_routing == nullptr || (_entrada_routing != nullptr && _entrada_routing->getNetmask() != Ipv4Address::ALLONES_ADDRESS)
                    || (entrada_routing != nullptr
                            && (msgsequencenumber > (entrada_routing->getSequencenumber())
                                    || (msgsequencenumber == (entrada_routing->getSequencenumber()) && numHops < (entrada_routing->getMetric()))))) {

                //remove old entry
                if (entrada_routing != nullptr)
                    rt->deleteRoute(entrada_routing);

                //adds new information to routing table according to information in hello message
                {
                    Ipv4Address netmask = Ipv4Address::ALLONES_ADDRESS; // Ipv4Address(par("netmask").stringValue());
                    ClusterAlgIpv4Route *e = new ClusterAlgIpv4Route();
                    e->setDestination(src);
                    e->setNetmask(netmask);
                    e->setGateway(next);
                    e->setInterface(interface80211ptr);
                    e->setSourceType(IRoute::MANET);
                    e->setMetric(numHops);
                    e->setSequencenumber(msgsequencenumber);
                    e->setExpiryTime(simTime() + routeLifetime);
                    rt->addRoute(e);
                }

                recHello->setNextAddress(source);
                numHops++;
                recHello->setHopdistance(numHops);
                double waitTime = intuniform(1, 50) / 100;
                packet->insertAtBack(recHello);
                sendDelayed(packet, waitTime, "ipOut");
                packet = nullptr;

            }
            delete packet;
            delete msg;
        }
        else
            throw cRuntimeError("Message arrived on unknown gate %s", msg->getArrivalGate()->getName());
    }
    else
        throw cRuntimeError("Message not supported %s", msg->getName());
}

} // namespace inet

