package inet.routing.cluster_alg;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.routing.cluster_alg.ClusterAlgRouter;
import inet.physicallayer.unitdisk.UnitDiskRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.environment.common.PhysicalEnvironment;
import inet.physicallayer.contract.packetlevel.IRadioMedium;

network ClusterAlgNetwork
{
    parameters:
        int numHosts;
        @display("bgb=650,650");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=-100,100;is=s");
        }
        radioMedium: <default("UnitDiskRadioMedium")> like IRadioMedium {
            parameters:
                @display("p=-100,200;is=s");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=-100,300;is=s");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=-100,400;is=s");
        }
        host[numHosts]: ClusterAlgRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
		visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=-100,500");
        }
        physicalEnvironment: PhysicalEnvironment {
			parameters:
            	@display("p=-100,600");
        }
        
    connections allowunconnected:
}
