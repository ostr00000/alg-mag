from pandas import Series


def printColumns(ser: Series, name: str = 'Column'):
    """generate class with all column to stdout"""
    print(f"class {name}:")
    indent = ' ' * 4
    for col in ser.columns:
        print(f"{indent}{col} = '{col}'")


class Column:
    run = 'run'
    type = 'type'
    module = 'module'
    name = 'name'
    attrname = 'attrname'
    attrvalue = 'attrvalue'
    value = 'value'
    count = 'count'
    sumweights = 'sumweights'
    mean = 'mean'
    stddev = 'stddev'
    min = 'min'
    max = 'max'
    binedges = 'binedges'
    binvalues = 'binvalues'
    vectime = 'vectime'
    vecvalue = 'vecvalue'