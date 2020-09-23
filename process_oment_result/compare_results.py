from __future__ import annotations

import re
from collections import defaultdict
from dataclasses import dataclass
from functools import cached_property, total_ordering
from typing import ClassVar, List

import numpy as np

from process_oment_result.config import outputPath


@total_ordering
@dataclass(unsafe_hash=True)
class CompareResult:
    algName: str
    value: float
    stdValue: float = 0.

    splitPattern: ClassVar = re.compile('[-,=]+')

    @cached_property
    def sortCode(self):
        """example:
        ('RcmdSimBigRange',
        'numHostP', '50',
        'speedP', '5',
        'leaderRepeatsP', '20',
        'routerNameP', 'ClusterAlgRouter')
        """
        val = tuple(self.splitPattern.split(self.algName))
        # val = val[0:5] + val[7:9] + val[5:7]
        newVal = []
        for i, maybeNum in enumerate(val):
            try:
                newVal.append(int(maybeNum))
            except ValueError:
                # newVal.append(maybeNum)
                pass

        return tuple(newVal)

    def __le__(self, other):
        assert isinstance(other, CompareResult)
        return self.sortCode <= other.sortCode

    @classmethod
    def filter(cls, results: List[CompareResult], name: str):
        return [r for r in results if r.algName.startswith(name)]


def findResults(resultPath=outputPath):
    for avrResult in resultPath.glob('*/pingRatio_average.txt'):
        dirName = str(avrResult.parent.name)
        with open(avrResult) as file:
            text = file.read()

        result = re.findall(r"value\W+(\d+(?:.\d+)?)", text)
        if not result:
            print(f"No result for {dirName}")
            continue

        algName = dirName
        value = float(result[0])
        stdValue = float(result[1])

        yield CompareResult(algName, value=value, stdValue=stdValue)


def findTime(resultPath = outputPath):
    for avrResult in resultPath.glob('*/pingStatistics_average.txt'):
        dirName = str(avrResult.parent.name)
        with open(avrResult) as file:
            text = file.read()

        result = re.findall(r"mean\W+(\d+(?:.\d+)?)", text)
        result2 = re.findall(r"stddev\W+(\d+(?:.\d+)?|inf)", text)
        if not (result and result2):
            print(f"No result for {dirName}")
            continue

        algName = dirName
        value = float(result[0])
        try:
            stdValue = float(result2[0])
        except ValueError:
            stdValue = 'inf'

        yield CompareResult(algName, value=value, stdValue=stdValue)


def printArgResultForParam(results: list, paramIndex: int, paramName: str, ):
    param = defaultdict(list)
    for result in results:
        param[result.sortCode[paramIndex]].append(result.value)

    for pValue, pResults in param.items():
        print(f'{paramName} [{pValue}] = {np.mean(pResults)}')


def statsForTimeParam(results: List[CompareResult]):
    results = CompareResult.filter(results, 'cmdTimeParam')
    if not results:
        return
    print('TimeParam')
    printArgResultForParam(results, 0, 'route entry expiry time')
    printArgResultForParam(results, 1, 'tc Interval')
    printArgResultForParam(results, 2, 'hello Interval')
    printArgResultForParam(results, 3, 'host Num')
    print()


def statsForParamRadius(results: List[CompareResult]):
    results = CompareResult.filter(results, 'paramRadius')
    if not results:
        return
    print('ParamRadius')
    printArgResultForParam(results, 0, 'range')
    printArgResultForParam(results, 3, 'host num')
    print()


def statsForRandomWayPointModel(results: List[CompareResult]):
    results = CompareResult.filter(results, 'cmdSimRandomWaypointModel')
    if not results:
        return
    print('RandomWayPointModel')
    printArgResultForParam(results, 0, 'density')
    printArgResultForParam(results, 1, 'host num')
    print()


def statsForParamSpeed(results: List[CompareResult]):
    results = CompareResult.filter(results, 'paramSpeed')
    if not results:
        return
    print('ParamSpeed')
    printArgResultForParam(results, 0, 'speed')
    printArgResultForParam(results, 1, 'host num')
    print()


def main():
    results, prec = sorted(findTime()), 3
    # results, prec = sorted(findResults()), 2
    maxAlgName = max(len(r.algName) for r in results)

    for result in results:
        textResult = f"{result.algName:{maxAlgName}} = {result.value:6.{prec}f}    " \
                     f"STD ={result.stdValue:5.{prec}f}  {result.stdValue/result.value:5.{prec}f}"
        print(textResult)
    print()

    statsForTimeParam(results)
    statsForParamRadius(results)
    statsForRandomWayPointModel(results)
    statsForParamSpeed(results)


if __name__ == '__main__':
    main()
