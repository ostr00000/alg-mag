package inet.routing.cluster_alg;

import inet.routing.contract.IManetRouting;

simple ClusterAlg like IManetRouting
{
    parameters:
        @display("i=block/routing");
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string netmask = default("255.255.0.0");  // netmask for the ad-hoc network
        double maxVariance = default(1); // This is the maximum of a random value to determine when the first hello message will be sent out
        bool manetPurgeRoutingTables = default(true);
        volatile double broadcastDelay @unit(s) = default(uniform(0s,0.01s));

        double helloInterval @unit(s) = default(5s);  // how often hello messages should be sent out
        double routeLifetime @unit(s) = default(9s);  // the expiry time of the routing entries

        double clusterUndecidedFirstTime @unit(s) = default(6s); // must be greater than helloIntrval
        double clusterUndecided @unit(s) = default(1s);
        double clusterMember @unit(s) = default(5s);
        double clusterLeader @unit(s) = default(3s);
        int maxLeaderRepeats = default(3);

        @signal[helloSignal](type="int");
        @signal[topologyControlSignal](type="int");
        @signal[stateChangedSignal](type="int");
        @signal[clusterDestroyedSignal](type="int");

        @statistic[helloNum](title="Number of HELLO messages"; source="helloSignal"; record=count,vector);
        @statistic[topologyControlNum](title="Number of Topolgy Control messages"; source="topologyControlSignal"; record=count,vector);
        @statistic[stateChangedNum](title="Number of changed state"; source="stateChangedSignal"; record=count,vector);
        @statistic[clusterDestroyedNum](title="Number of destroyed cluster"; source="clusterDestroyedSignal"; record=count,vector);


    gates:
        input ipIn;
        output ipOut;
}

