package inet.routing.cluster_alg;

import inet.routing.contract.IManetRouting;

simple ClusterAlg like IManetRouting
{
    parameters:
        @display("i=block/routing");
        string interfaceTableModule = default("^.interfaceTable"); 
        string routingTableModule = default("^.ipv4.routingTable");
        bool manetPurgeRoutingTables = default(true);
        string networkProtocolModule = default("^.ipv4.ip");


        double maxVariance = default(2); // This is the maximum of a random value to determine when the first hello message will be sent out
        volatile double broadcastDelay @unit(s) = default(uniform(0s,0.01s));

        double helloInterval @unit(s) = default(4s);  // how often hello messages should be sent out
        double tcInterval @unit(s) = default(7s);  // how often topology control messages should be sent out
        double routeLifetime @unit(s) = default(8.1s);  // the expiry time of the routing entries

        double clusterUndecidedFirstTime @unit(s) = default(6s); // must be greater than helloIntrval
        double clusterUndecided @unit(s) = default(1s);
        double clusterMember @unit(s) = default(5s);
        double clusterLeader @unit(s) = default(3s);
        int maxLeaderRepeats = default(3);

        @signal[helloSignal](type="long");
        @signal[topologyControlSignal](type="long");
        @signal[stateChangedSignal](type="long");
        @signal[clusterDestroyedSignal](type="long");

        @statistic[helloNum](title="Number of HELLO messages"; source="helloSignal"; record=count,vector);
        @statistic[topologyControlNum](title="Number of Topolgy Control messages"; source="topologyControlSignal"; record=count,vector);
        @statistic[stateChangedNum](title="Number of changed state"; source="stateChangedSignal"; record=count,vector);
        @statistic[clusterDestroyedNum](title="Number of destroyed cluster"; source="clusterDestroyedSignal"; record=count,vector);

		@signal[messageSendSignal](type="long");
		@signal[messageTransferedSignal](type="long");
		@signal[messageReceivedSignal](type="long");
		
		@staictic[messageSendNum](title="Number of send messages"; source="messageSendSignal"; record=count,vector);
		@staictic[messageTransferedNum](title="Number of transfered messages"; source="messageTransferedSignal"; record=count,vector);
		@staictic[messageReceivedNum](title="Number of received messages"; source="messageReceivedSignal"; record=count,vector);
		
    gates:
        input ipIn;
        output ipOut;
}

