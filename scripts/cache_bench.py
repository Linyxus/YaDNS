import argparse
from hash import Rec, load_rec, Domain
from time import perf_counter_ns
from typing import Optional
from subprocess import run
from concurrent.futures.thread import ThreadPoolExecutor
from concurrent.futures import as_completed

from typing import List

def undomain(domain: Domain) -> str:
    return '.'.join(domain)

def compose_cmd(domain: Domain) -> str:
    d = undomain(domain)
    return ['dig', '+retry=0', '@127.0.0.1', d]

def resolve(domain: Domain) -> Optional[int]:
    start = perf_counter_ns()
    cmd = compose_cmd(domain)
    ret = run(cmd, capture_output=True)
    if ret.returncode == 0:
        return perf_counter_ns() - start
    else:
        return None


def benchmark(domains: List[Domain]) -> float:
    clean_results = []
    for domain in domains:
        clean_results.append(resolve(domain))
    num_failure = sum(1 for r in clean_results if r is None)
    clean_results = [r for r in clean_results if r is not None]
    avg_ms = sum(x / 1e8 for x in clean_results) / len(clean_results)

    return avg_ms
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--hosts', type=str, default='hosts.txt')
    parser.add_argument('--output', type=str, default=None)
    parser.add_argument('--job', type=int, default=2)
    args = parser.parse_args()

    rec = load_rec(args.hosts)
    domains = [r.domain for r in rec if r.ip != '0.0.0.0']

    avg_ms = []
    with ThreadPoolExecutor(max_workers=args.job) as e:
        futures = [e.submit(benchmark, domains) for _ in range(args.job)]
        for future in as_completed(futures):
            avg_ms.append(future.result())

    avg_ms = sum(avg_ms) / len(avg_ms)

    if args.output is not None:
        with open(args.output, 'a') as f:
            f.write(f'{avg_ms}\n')
    else:
        print(avg_ms)
