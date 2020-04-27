package inet.routing.cluster_alg;

import inet.node.inet.AdhocHost;
import inet.routing.cluster_alg.ClusterAlg;

network ClusterAlgRouter extends AdhocHost
{
    submodules:
        clusterAlg: ClusterAlg {
            @display("p=825,226");
        }
    connections:
        clusterAlg.ipOut --> tn.in++;
        clusterAlg.ipIn <-- tn.out++;
}

