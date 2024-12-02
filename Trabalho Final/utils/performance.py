import subprocess
import json
import time


def runMedian(path, offset):
    total_exec = 3
    median = 0

    for i in range(total_exec):
        result = subprocess.run([path, offset],
                                capture_output=True)
        time.sleep(1)

        result = result.stdout.decode('utf-8')
        result = result.split(": ")[1].strip("s\n")

        median += float(result)

    median /= total_exec
    return round(median, 6)


def runTests(path):

    offsets = [1, 10, 10000, 1000000, 10000000]
    results = []

    for offset in offsets:

        print(f'\nRunning Test in offset: {offset}')

        median = runMedian(path, str(offset))

        results.append({"median": median,
                        "offset": offset})

        writeToFile(results)


def writeToFile(results):
    with open('out.json', 'w', encoding='utf-8') as f:
        json.dump(results, f, ensure_ascii=False, indent=4)


def main(path):
    runTests(path)


if __name__ == '__main__':
    main("./../bbp-algo")
