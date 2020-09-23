from pathlib import Path
from typing import List, TextIO

import numpy as np
from matplotlib import pyplot as plt
from pandas import DataFrame

from process_oment_result.config import SIMULATION_TIME, SHOW_IMAGE


class Drawer:

    def __init__(self, outputDir: Path):
        self.outputDir = outputDir
        self.outputDir.mkdir(parents=True, exist_ok=True)
        self.filename = None

    def plotSum(self, column, text, binsNum=SIMULATION_TIME + 1, binsStep=10):
        avr = []
        for row in column:
            avr.append(Drawer._extract(row))

        mat = np.hstack(avr) if avr else avr
        self.basePlot(mat, text, binsNum, binsStep)

    @staticmethod
    def _extract(singleValueRow):
        vec = np.array([float(v) for v in singleValueRow.split()])
        return vec

    def basePlot(self, mat, text, binsNum=SIMULATION_TIME + 1, binsStep=50):
        bins = range(0, binsNum, binsStep)
        plt.clf()
        plt.hist(mat, bins)

        plt.ylabel(text)
        plt.xlabel('Czas symulacji [s]')

        self._endPlot()

    def _endPlot(self):
        if self.filename is not None:
            plt.savefig(self.outputDir / self.filename, dpi=200)

        if SHOW_IMAGE:
            plt.ion()
            plt.show()
            plt.ioff()

    def drawTable(self, colLabels: List[str], cellValues, average=''):
        if average:
            averageColumn = np.array(cellValues)[:, 1]
            averageColumn = np.array(averageColumn, dtype=float)
            averageVal = np.average(averageColumn)
            cellValues.append([average, f'{averageVal:.2f}'])

        # plt.clf()
        # plt.ioff()
        # fig, ax = plt.subplots(figsize=(4, 8))  # type: (Figure, Axes)
        # fig.patch.set_visible(False)
        # ax.axis('off')
        # ax.axis('tight')

        with open(self.outputDir / (self.filename + '_val.txt'), 'w') as file:
            print(' | '.join(colLabels))
            print(' | '.join(colLabels), file=file)
            for name, value in cellValues:
                print(f"{name} = {value}")
                print(f"{name} = {value}", file=file)

        # plt.table(colLabels=np.array(colLabels, dtype=str),
        #           cellText=cellValues, loc='center', cellLoc='left')
        # fig.tight_layout()
        #
        # self._endPlot()
        # plt.close(fig)

    def table(self, extractedData: DataFrame, name: str):
        filename = self.filename

        self.filename = filename + '_all.txt'
        self._table(extractedData, name)

        mean = extractedData.mean(axis=0)
        std = extractedData.std(axis=0)
        self.filename = filename + '_average.txt'
        self._table(mean, name, std)

    def _table(self, arr, name, std=None):
        with open(self.outputDir / self.filename, 'w') as file:
            self.filePrint(name, file=file)
            self.filePrint(arr.to_string(), file=file)
            if std is not None:
                self.filePrint("Std:", file=file)
                self.filePrint(std, file=file)

    @staticmethod
    def filePrint(*args, file: TextIO = None, **kwargs):
        print(*args, **kwargs)
        if file is not None:
            print(*args, **kwargs, file=file)
