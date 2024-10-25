import subprocess
import json

def runMedian(path, offset, threads, batch_size):

    total_exec = 5
    median = 0

    for i in range(total_exec):
        result = subprocess.run([path, offset, threads, batch_size],
                                capture_output=True)
        result = result.stdout.decode('utf-8')
        result = result.split(": ")[1].strip("s\n")

        median += float(result)

    median /= total_exec
    return round(median, 6)


def runTests(path):

    #offsets = [1, 10, 10000, 1000000, 10000000]
    offsets = [100000000]
    threads = [1, 2, 4, 8, 12]
    results = []
    
    for offset in offsets:
        
        print(f'\nRunning Test in offset: {offset}')

        for thread in threads:

            batch_size = 1
            print(f'thread(s): {thread}')

            while batch_size <= offset:

                print(f'batch Size: {batch_size}')

                median = runMedian(path, 
                                   str(offset),
                                   str(thread),
                                   str(batch_size))

                results.append({"median": median,
                                "offset": offset,
                                "threads": thread,
                                "batch_size": batch_size})

                batch_size *= 10

    return results


def writeToFile(results):
    with open('out_large.json', 'w', encoding='utf-8') as f:
        json.dump(results, f, ensure_ascii=False, indent=4)


def main(path):

    results = runTests(path)
    writeToFile(results)


if __name__ == '__main__':
    main("./../bbp-algo-conc-custom-batch-size")
