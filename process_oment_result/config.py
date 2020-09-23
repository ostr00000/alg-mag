from pathlib import Path

SIMULATION_TIME = 200
SHOW_IMAGE = False

projectPath = Path(
    '/home/ostro/przerzut/studia/mag/omnet/workspace/inet4/src/inet/routing/cluster_alg/results')

outputPath = Path('.').absolute() / 'output'
outputPath.mkdir(parents=True, exist_ok=True)
