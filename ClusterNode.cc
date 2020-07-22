/*
 * ClusterNode.cpp
 *
 *  Created on: Jul 17, 2020
 *      Author: ostro
 */

#include "inet/routing/cluster_alg/ClusterNode.h"

namespace inet {

std::string ClusterNode::getTextRepresentation(ClusterNode *node)
{
    Ipv4Address address;
    if (node->clusterInfo != nullptr) {
        address = node->clusterInfo->clusterId;
    }
    else if (node->clusterRoute != nullptr) {
        address = node->clusterRoute->getIdFromSource();
    }
    else if (node->address != Ipv4Address::UNSPECIFIED_ADDRESS) {
        address = node->address;
    }
    else {
        throw cRuntimeError("unknown ClusterNode type");
    }
    auto text = address.str();
    std::replace(text.begin(), text.end(), '.', '_');
    return "Node" + text;
}

std::string ClusterNode::toString(cTopology *topology)
{
    std::stringstream stream;
    stream << "digraph graphname {\n";

    for (int i = 0, j = topology->getNumNodes(); i < j; i++) {
        ClusterNode *node = dynamic_cast<ClusterNode*>(topology->getNode(i));
        stream << getTextRepresentation(node) << ";\n";
    }

    stream << "\n";
    for (int i = 0, j = topology->getNumNodes(); i < j; i++) {
        ClusterNode *node = dynamic_cast<ClusterNode*>(topology->getNode(i));
        auto fromNodeText = getTextRepresentation(node);

        for (int k = 0, l = node->getNumOutLinks(); k < l; k++) {
            stream << fromNodeText << " -> ";
            cTopology::LinkOut *link = node->getLinkOut(k);
            ClusterLink* cl = (ClusterLink*)(link);
            std::string label = " [ label = \"" + cl->relation + "\" ]";
            ClusterNode *otherNode = dynamic_cast<ClusterNode*>(link->getRemoteNode());
            stream << getTextRepresentation(otherNode) << label << "\n";

        }
    }

    stream << "}";
    return stream.str();
}
}
