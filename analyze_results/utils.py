import os, sys, math
from decimal import Decimal

def binomial(n, k):
    top = bot = 1
    for i in range(k):
        top *= n - i
        bot *= i + 1
    return top // bot


def fac(n):
    assert n >= 0
    res = 1
    while n > 1:
        res *= n
        n -= 1
    return res


def listdir(f):
    return [f.rstrip("/") + "/" + ff for ff in os.listdir(f)]


def collect_todo(files):
    fnames = []
    for file in files:
        if os.path.isdir(file):
            for f in os.listdir(file):
                fnames.append(os.path.join(file, f))
        elif os.path.isfile(file):
            fnames.append(file)
        else:
            raise RuntimeError(f"file problem with {file}")

    keys = "speck rounds split cw cx cy dx dy".split()

    todo = []
    for path in fnames:
        fname = os.path.basename(path)
        if fname.startswith("speck"):
            s = fname.rstrip("txt").rstrip(".")
            s = s.split("_")
            info = {}
            for kv in s:
                for k in keys:
                    if kv.startswith(k):
                        info[k] = kv[len(k):]
                        if k in ("dx", "dy", "cx", "cy"):
                            pass
                        elif k == "split":
                            info[k] = tuple(map(int, info[k].split("-")))
                        else:
                            info[k] = int(info[k], 10)
                        break
                else:
                    raise Exception((path, kv))

            todo.append((path, info))
    return todo


def readPath(path,info):
    with open(path) as f:
        trail_path = f.readline().strip()
        sampled_n_pairs, sampled_n_trails = map(int, f.readline().split())
        l = f.readline()
        cluster_image, qinv_log = l.split()

        qinv_log = Decimal(qinv_log)
        cluster_image = int(cluster_image)

        qinv = 2**qinv_log
        assert qinv != 0

        q = 2**(-qinv_log)

        word_size = info["speck"]//2

        print("=====================")
        print("Speck%d/%d" % (2*word_size, 4*word_size))
        print("Rounds =", info["rounds"])
        print("Number of sampled pairs =", sampled_n_pairs, f"= 2^{math.log(sampled_n_pairs, 2)}")
        print("Number of sampled trails =", sampled_n_trails, f"= 2^{math.log(sampled_n_trails, 2)}")
        print(f"Cluster size, |S| = {cluster_image} = 2^{math.log(cluster_image, 2):.2f}")
        print("Efficiency, q %.2f" % q, f"= 2^{math.log(q, 2)}")

        weightCount = []
        for _ in range(4*word_size):
            weightCount.append(list(map(int, f.readline().split())))
            assert sum(weightCount[-1]) == sampled_n_trails

        totalProb = weightCount_to_totalProb(weightCount, word_size=word_size)
    return sampled_n_pairs, sampled_n_trails, cluster_image, q, totalProb


def find_n_pairs(Dq, C, success):
    p = Decimal(1.0)/Decimal(Dq)
    ip = 1 - p
    l = 1
    r = 2**100

    success = Decimal(success)

    best = 999, Decimal(0.0)
    while l < r:
        mid = (l + r) // 2
        pC = 1 - sum(binomial(mid, i) * p**i * ip**(mid - i) for i in range(C))
        if abs(pC - success) < abs(best[1] - success):
            best = mid, pC
            if abs(pC - success) < 1e-3:
                break

        if pC < success:
            l = mid + 1
        else:
            r = mid - 1
    return best[0], float(best[1])


def weightCount_to_totalProb(weightCount, word_size):
    n_trails = sum(weightCount[-1])
    assert(sum(row) == n_trails for row in weightCount)
    if n_trails == 0:
        return None

    totalProb = [Decimal(0.0)] * (4*word_size)
    for rno in range(4):
        for pos in range(word_size):
            prob = 0
            for wt, wc in enumerate(weightCount[rno*word_size+pos]):
                prob += wc * Decimal(2.0)**-wt
            totalProb[rno*word_size+pos] = prob / n_trails
    totalProb = [Decimal(1.0)] + totalProb
    return totalProb
