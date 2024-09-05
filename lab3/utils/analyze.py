import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

def parseResults(text):
    data = {}
    resultsArr = text.split("\n\n")

    for result in resultsArr:

        test_dict = {}

        # Check if it's a non empty line
        if (result == ""):
            continue

        # Parse Header Info
        lines = result.splitlines()
        matrix_size, rhs = lines[0].strip().split("-", 1)
        median, type_test = rhs.strip().replace("Median Values", "").split(" ", 1)
        type = type_test.strip("() ")

        if (type == "seq"):
            threads = "0"
        else:
            type, threads = type.replace(" Threads", "").split(" - ")

        # Parse Time Info
        read = lines[1].replace("Read: ", "")
        write = lines[2].replace("Write: ", "")
        mult = lines[3].replace("Mult.: ", "")
        total_time = lines[4].replace("Total Time: ", "")

        # Add to Dict
        test_dict["type"] = type
        test_dict["median"] = int(median)
        test_dict["threads"] = int(threads)
        test_dict["read"] = float(read)
        test_dict["write"] = float(write)
        test_dict["mult"] = float(mult)
        test_dict["total_time"] = float(total_time)

        if matrix_size in data:
            data[matrix_size].append(test_dict)
        else:
            data[matrix_size] = [test_dict]

    return data


def calcPerformance(tests):
    result = {}
    seq_time = -1

    # Get test with one thread
    for test in tests:
        if test["threads"] == 1:
            seq_time = test["total_time"]
            break

    # Checks if thre's is such test
    if not seq_time:
        return None

    # Calc performance for each result
    for test in tests:
        if test["type"] != "seq":
            result[test["threads"]] = seq_time / test["total_time"]

    return result


def calcEfficiency(performances):

    result = {}

    # Calc efficiency
    for p in performances:
        result[p] = (performances[p] / p) * 100

    return result


def makeGraphs(performance, efficiency):

    dtPerformance = pd.DataFrame.from_dict(performance, orient='index')
    dtEfficiency = pd.DataFrame.from_dict(efficiency, orient='index')

    print(dtPerformance)
    #print(dtEfficiency)

    x = np.arange(len(performance))
    width = 0.25
    multiplier = 0

    print(performance.keys)

    fig, axis = plt.subplots(2, 1, layout='constrained')

    dtPerformance.plot(ax=axis[0],
                       kind='bar',
                       stacked=False)
    dtEfficiency.plot(ax=axis[1],
                      kind='bar',
                      stacked=False)

    axis[0].set_title("Aceleração",
                      fontsize=15,
                      fontweight='bold',
                      pad=10)

    axis[0].tick_params(axis='x', rotation=0)

    axis[1].tick_params(axis='x', rotation=0)
    axis[1].set_title("Eficiência",
                      fontsize=15,
                      fontweight='bold',
                      pad=10)

    plt.show()


def main(path):
    fd = open(path, "r")
    text = fd.read()

    result_dict = parseResults(text)
    performance_dict = {}
    efficiency_dict = {}

    for matrix_size in result_dict:
        matrix_list = result_dict[matrix_size]
        performance_dict[matrix_size] = calcPerformance(matrix_list)
        efficiency_dict[matrix_size] = calcEfficiency(performance_dict[matrix_size])

    makeGraphs(performance_dict, efficiency_dict)


if __name__ == "__main__":
    if len(sys.argv) == 2:
        main(sys.argv[1])
    else:
        print("Usage: python [path-to-file]")
