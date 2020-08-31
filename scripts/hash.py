import argparse
from collections import namedtuple
from typing import List

Rec = namedtuple('Rec', ['ip', 'domain'])

Domain = List[str]

def load_rec(path: str) -> List[Rec]:
    content = open(path, 'r').read()
    lines = [x for x in content.strip().split('\n') if len(x)]
    def rec(line):
        ip, domain = line.strip().split(' ')
        return Rec(ip=ip, domain=domain.split('.'))
    return [rec(x) for x in lines]

def count_items(xs):
    ret = dict()
    for x in xs:
        if x in ret:
            ret[x] = ret[x] + 1
        else:
            ret[x] = 1
    return [(k, x) for k, x in ret.items()]

def hash_domain(domain: Domain) -> int:
    if len(domain) == 0:
        return 1

    # mod = 2 ** 16
    mod = 2 ** 8
    ret = 0
    name = domain[0]
    for x in name:
        ret = ret * 31 + ord(x)
        ret = ret % mod
    return (ret ^ hash_domain(domain[1:])) % mod

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--hosts', type=str, default='hosts.txt')
    args = parser.parse_args()

    # load hosts and calculate hash for each domain
    hashes = [hash_domain(x.domain) for x in load_rec(args.hosts)]
    res = count_items(hashes)
    s = sum(x[1] - 2 for x in res if x[1] > 2)
    print(f'duplicate tot {s}')
    print(res)

    num_use = 0
    for k, x in res:
        num_use += 2 if x > 2 else x
    print(f'{num_use / 512:.4f}')

if __name__ == '__main__':
    main()