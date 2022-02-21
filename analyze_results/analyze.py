import argparse
from decimal import Decimal, getcontext, Overflow

import utils


# using decimal is not necessary,
# but can ensure more precise calculations
getcontext().prec = 150  # precision for floats

ln2 = Decimal(2).ln()
log2 = lambda v: Decimal(v).ln() / ln2

parser = argparse.ArgumentParser(
    description="""
Sample cmd:
$ time python3 analyze.py . > log.out
""".strip()
)
parser.add_argument('files', metavar='file/dir', type=str, nargs='+',
                    help='Files/dirs to analyze')
args = parser.parse_args()


def main():
    todo = utils.collect_todo(args.files)
    for itr, (path, info) in enumerate(todo):
        process_trace(path, info)


def process_trace(path, info):
    n_rounds = info["rounds"]
    word_size = info["speck"]//2

    split = info["split"]
    assert len(split) == 4
    assert split[0] == 1
    assert split[-1] == 2

    sampled_n_pairs, sampled_n_trails, cluster_image, Dq, totalProb \
        = utils.readPath(path,info)

    problogs = [-log2(v) for v in totalProb]

    datatime_by_C = {}

    # Print results for up to c=8
    for C in range(1, 9):
        print("=====================")
        print("C =", C)
        print("=====================")

        n_pairs, pC = utils.find_n_pairs(Dq=Dq, C=C, success=Decimal(0.63212))
        n_pairs_log = log2(n_pairs)

        c_prime = Decimal(n_pairs) / Decimal(Dq)
        print(f"c' = {c_prime:.5f} = 2^{log2(c_prime):.5f}")

        n_trails = n_pairs * sampled_n_trails / sampled_n_pairs
        n_trails_log = log2(n_trails)
        print("Number of pairs = 2^%.2f" % n_pairs_log )
        print("Number of trails = 2^%.2f" % n_trails_log )
        print(f"Success chance = {pC*100:.2f}%")

        v_c1 = [
            # n_trails * 2^d * q_d
            Decimal(d - q_d_weight + n_trails_log)
            for d, q_d_weight in enumerate(problogs)
        ]

        if C > 1:
            # v_d in the paper
            v = []
            for d, kelog in enumerate(v_c1):
                alpha = Decimal(2**(kelog - d)) # = ntrails * prob
                try:
                    sub = alpha.exp() - sum(alpha**i / utils.fac(i) for i in range(C-1))
                    v.append(kelog - alpha / ln2 + log2(sub))
                except (OverflowError, Overflow):
                    # large alpha: no effect of polynomial sum on the exponent
                    # log2(sub) ~= alpha / ln 2
                    v.append(kelog)
        else:
            v = v_c1

        n_trailkeys_nonfinal = sum(2**c for c in v[:-1])
        n_trailkeys_final = 2**v[-1]

        T_enum = n_trailkeys_nonfinal * 2 / n_rounds
        print(f"Number of trail*keys = 2^{log2(n_trailkeys_nonfinal):.3f}")

        no_trail = (split[1] != 0)

        if no_trail:
            # For (0+r)
            # => no diff. trail for full path
            # need to perform full decryption to filter candidates.
            T_trials = n_trailkeys_final * (n_rounds - 4) / Decimal(n_rounds)
            print("Key check costs R-4 rounds per candidate subkey / ct/pt pair")
        else:
            # For (1+r)
            # => have diff. trail for full path
            # can check round-by round,
            # but after 1 round most of the candidates should be filtered.
            T_trials = n_trailkeys_final * 2 / Decimal(n_rounds)
            print("Key check costs 2 rounds per candidate subkey / ct/pt pair")

        T_cnt_log = log2(T_enum + T_trials)
        T_enum_log = log2(T_enum)
        T_trials_log = log2(T_trials)

        if word_size == 16:
            coef_a = Decimal(2)**Decimal(12.1)
            Ta = Decimal(1) / (5*n_rounds)
            Tb = Decimal(11) / (5*n_rounds)
        elif word_size == 32:
            coef_a = Decimal(2)**Decimal(25.03)
            Ta = Decimal(3) / (5*n_rounds)
            Tb = Decimal(11) / (5*n_rounds)
        else:
            raise NotImplementedError()

        # generic MiF
        T_mif = n_pairs * coef_a * (Ta + cluster_image * Tb / Decimal(2)**word_size)
        if split[-1] == 2:
            # simplified procedure (2R MiF)
            alt = 2**n_pairs_log * cluster_image * 4 / Decimal(n_rounds)
            T_mif = min(T_mif, alt)

        T_mif_log = log2(T_mif)

        T_att = T_enum + T_trials + T_mif
        T_att_log = log2(T_att)

        print(f"#keys to test = 2^{v[-1]:.2f}")
        print(f"Total key recovery complexity, T_cnt = 2^{T_cnt_log:.2f}")
        print(f"Total MiF complexity, T_mif = 2^{log2(T_mif):.2f}")
        print(f"Overall attack complexity (KR + MiF complexities), T_att = 2^{T_att_log:.2f}")

        datatime_by_C[C] = (
            n_pairs_log, n_trails_log, T_cnt_log,
            T_mif_log, T_enum_log, T_trials_log, T_att_log,
        )
        print()

    print("===================================================================")
    print(f"Summary for {n_rounds}-round Speck32 attack")
    print("D denotes data complexity in the number of chosen plaintexts")
    print("T denotes time complexity in the number of full encryptions")
    print("===================================================================")
    print("C\tD (log2)\tT_mif (log2)\tT_kr (log2)\tT_att(log2)")
    for C, data in datatime_by_C.items():
        (
            n_pairs_log, n_trails_log, T_cnt_log,
            T_mif_log, T_enum_log, T_trials_log, T_att_log,
        ) = data
        print(f"{C:d}\t{n_pairs_log+1:.2f}\t\t{T_mif_log:.2f}\t\t{T_cnt_log:.2f}\t\t{T_att_log:.2f}")
    print("===================================================================")
    print()


if __name__ == '__main__':
    main()
