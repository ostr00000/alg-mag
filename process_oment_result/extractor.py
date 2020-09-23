import re
import sys
from pathlib import Path
from typing import List

from decorator import decorator
from pandas import DataFrame

from process_oment_result.columns import Column
from process_oment_result.drawer import Drawer


@decorator
def saveExecutionDec(fun, *args, **kwargs):
    try:
        return fun(*args, **kwargs)
    except Exception as exc:
        print(exc, file=sys.stderr)
        # raise exc


class Extractor:
    CONVERT_CLUSTER_ALG = re.compile(r'ClusterAlgNetwork\.host\[(\d+)\]\.clusterAlg')
    CONVERT_CLUSTER_APP = re.compile(r'ClusterAlgNetwork\.host\[(\d+)\]\.app\[0\]')

    @classmethod
    def _convertToHostNum(cls, text):
        if not isinstance(text, str):
            return text

        match = cls.CONVERT_CLUSTER_ALG.match(text)
        if not match:
            match = cls.CONVERT_CLUSTER_APP.match(text)
            if not match:
                return text

        try:
            return int(match[1])
        except ValueError:
            return match[1]

    def __init__(self, data: DataFrame, outputDir: Path):
        data = data.copy()
        data[Column.module] = data[Column.module].apply(self._convertToHostNum)
        self.data = data
        self.drawer = Drawer(outputDir)

    @saveExecutionDec
    def topologyControlNum(self):
        extractedData = self._extract('topologyControlNum:vector')
        self._plotSum(extractedData, 'Wiadomości Topology Control', 'tc')

    @saveExecutionDec
    def helloNum(self):
        extractedData = self._extract('helloNum:vector')
        self._plotSum(extractedData, "Wiadomości Hello", 'hello_msg')

    @saveExecutionDec
    def stateChangedNum(self):
        extractedData = self._extract('stateChangedNum:vector')
        self._plotSum(extractedData, "Zmiana stanu węzła", 'state_change')

    @saveExecutionDec
    def clusterDestroyedNum(self):
        extractedData = self._extract('clusterDestroyedNum:vector')
        self._plotSum(extractedData, "Zniszczenie klastra", 'destroyedCluster')

    def _extract(self, columnName: str, columnType: str = 'vector', columns: List[Column] = None):
        if columns is None:
            columns = [Column.module, Column.vectime]
        assert Column.module in columns, "Required column 'module'"

        filterExp = ((self.data[Column.name] == columnName)
                     & (self.data[Column.type] == columnType))
        extractedData = self.data[filterExp][columns]
        sortedData = extractedData.sort_values(by=Column.module)
        return sortedData

    def _plotSum(self, extractedData: DataFrame, imageText: str, filename: str):
        self.drawer.filename = filename + '_full.png'
        vector = extractedData[Column.vectime]
        self.drawer.plotSum(vector, imageText)

        # self.drawer.filename = filename + '_step5.png'
        # self.drawer.plotSum(vector, imageText,
        #                     binsNum=200, binsStep=5)

    @saveExecutionDec
    def successPing(self):
        transmitPing = self._extract('pingTxSeq:count', 'scalar', [Column.module, Column.value])
        receivedPing = self._extract('pingRxSeq:count', 'scalar', [Column.module, Column.value])

        ratios = {Column.module: [], Column.value: []}
        for nodeNumber, tp, rp in zip(transmitPing[Column.module], transmitPing[Column.value],
                                      receivedPing[Column.value]):
            ratios[Column.module].append(float(nodeNumber))
            ratios[Column.value].append(rp / tp * 100)

        if ratios[Column.value]:
            dataFrame = DataFrame(data=ratios)
            self.drawer.filename = "pingRatio"
            self.drawer.table(dataFrame, 'Udane wysłanie i odbiór wiadomości ping')

    @saveExecutionDec
    def pingStatistics(self):
        extractedData = self._extract(
            'rtt:histogram', columnType='histogram',
            columns=[Column.module, Column.mean, Column.stddev, Column.min, Column.max])

        self.drawer.filename = 'pingStatistics'
        self.drawer.table(extractedData, name="Ping round-trip time")
