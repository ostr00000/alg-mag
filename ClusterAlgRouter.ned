package inet.routing.cluster_alg;

import inet.node.inet.AdhocHost;
import inet.routing.cluster_alg.ClusterAlg;
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
        queue: PacketQueue {
            @display("p=900,376");
        }

        server: TokenBasedServer {
            @display("p=1000,376");
            initialNumTokens = 1;
        }
        gate: PacketGate {
            @display("p=1100,376");
            initiallyOpen = true;
        }
        tokenGenerator: PacketBasedTokenGenerator {
            @display("p=300,376");
            serverModule = "^.server";
        }
    connections:
        clusterAlg.ipOut --> queue.in;
		queue.out --> server.in;
  		server.out --> gate.in;
  		gate.out --> tn.in++; 

        clusterAlg.ipIn <-- tn.out++;
        tokenGenerator.in <--tn.out++;
}

