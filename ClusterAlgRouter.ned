//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

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

