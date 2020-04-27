import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;


namespace inet;

enum MessageType {
	HELLO=1;
	TOPOLOGY_CONTROL=2;
};


class ClusterAlgBase extends FieldsChunk{
    MessageType messageType;
	Ipv4Address srcAddress; // not used
    Ipv4Address nextAddress;//not used
    unsigned int sequencenumber;
    int hopdistance;
    Ipv4Address srcId;
}


enum NodeState {
 	   UNDECIDED = 1;
 	   LIDER = 2;
 	   MEMBER = 3;
}

class ClusterAlgHello extends ClusterAlgBase {
    int neighborsNum;
    int undecidedNeighborsNum;
    NodeState state;
    Ipv4Address clusterHeadId;
    Ipv4Address neighbors[];
}

class ClusterAlgTopologyControl extends ClusterAlgBase {
	    
}

