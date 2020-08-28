package inet.routing.cluster_alg;

import inet.node.inet.AdhocHost;
import inet.routing.cluster_alg.ClusterAlg;
import inet.routing.cluster_alg.QueueSequentialTransmission;
import inet.queueing.queue.PacketQueue;
import inet.queueing.common.PacketGate;
import inet.queueing.server.TokenBasedServer;
import inet.queueing.tokengenerator.PacketBasedTokenGenerator;

network ClusterAlgRouter extends AdhocHost
{
    submodules:
        clusterAlg: ClusterAlg {
            @display("p=800,376");
        }
        queue: QueueSequentialTransmission {
            @display("p=900,376");
        }

    connections:
        clusterAlg.ipOut --> queue.in;
        queue.out --> tn.in++;
      
        clusterAlg.ipIn <-- tn.out++;
}

