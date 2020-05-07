package inet.routing.cluster_alg;

import inet.routing.contract.IManetRouting;

simple ClusterAlg like IManetRouting
{
    parameters:
        @display("i=block/routing");
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        double helloInterval @unit(s) = default(5s);  // how often hello messages should be sent out
        string netmask = default("255.255.0.0");  // netmask for the ad-hoc network
        double maxVariance = default(1); // This is the maximum of a random value to determine when the first hello message will be sent out
        double routeLifetime @unit(s) = default(5s);  // the expiry time of the routing entries
        bool manetPurgeRoutingTables = default(true);
        volatile double broadcastDelay @unit(s) = default(uniform(0s,0.01s));
    
    	double clusterUndecidedFirstTime @unit(s)= default(6s); // must be greater than helloIntrval
        double clusterUndecided @unit(s)= default(1s);
		double clusterMember @unit(s)= default(5s);
		double clusterLeader @unit(s)= default(5s);
		
	gates:
        input ipIn;
        output ipOut;
}

