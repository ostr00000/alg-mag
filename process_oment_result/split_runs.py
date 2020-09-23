from collections import defaultdict
from typing import Tuple, Iterable

from pandas import DataFrame, Series, concat

from process_oment_result.columns import Column


def splitRun(dataFrame: DataFrame) -> Iterable[Tuple[str, DataFrame]]:
    runColumn: Series = getattr(dataFrame, Column.run)
    runNames = runColumn.unique()

    if len(runNames) == 1:
        yield '', dataFrame
        return

    runNameToRuns = defaultdict(list)
    for runName in runNames:
        oneRun = dataFrame[dataFrame[Column.run] == runName]
        cond = ((oneRun[Column.attrname] == 'iterationvarsf')
                & (oneRun[Column.type] == 'runattr'))
        runName = oneRun.loc[cond][Column.attrvalue].values[0]
        runNameToRuns[runName].append(oneRun)

        # yield runName, oneRun

    for runName, runs in runNameToRuns.items():
        runs = concat(runs)
        yield runName, runs
