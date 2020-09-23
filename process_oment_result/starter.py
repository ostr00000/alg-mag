import pandas as pd

from process_oment_result.config import projectPath, outputPath
from process_oment_result.extractor import Extractor
from process_oment_result.split_runs import splitRun


def main():
    alreadyProcessed = [file.name for file in outputPath.iterdir()]
    for file in projectPath.glob('*.csv'):
        try:
            data = pd.read_csv(str(file))
        except IOError as error:
            print(f"skipping file: {file} - reason: {error}")
            continue

        for runName, runData in splitRun(data):
            outputDir = outputPath / f'{file.name.replace(".csv", "")}_{runName}'
            if outputDir.name in alreadyProcessed:
                print(f"{outputDir.name} - skipping, already processed")
                continue
            else:
                print(str(outputDir))

            ex = Extractor(runData, outputDir)

            ex.topologyControlNum()
            ex.helloNum()
            ex.stateChangedNum()
            ex.clusterDestroyedNum()
            ex.successPing()
            ex.pingStatistics()


if __name__ == '__main__':
    main()
