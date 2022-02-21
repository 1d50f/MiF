# Practical attacks on 11 and 12 rounds of Speck32/64 

## How to run

Success rate: 63%

- 11-round attack time: 2-3 seconds
- 12-round attack time: < 15 minutes

```bash
$ make test_attack11
$ make test_attack12
````

## Example 11 rounds

```bash
$ make test_attack11
time ./attack11.elf 26.1 3
n_rounds: 11
seed: 2010985740 = 0x77dd350c
c: 3
n_pairs: 71925499 = 2^26.10
[DEBUG] Secret master key: 322d cb17 f68f f089  | round keys: 322d a977 6cbb 3273 70f9 ff6a c1c5 2a26 e000 3c73 6f08 

Stage 0+1 (data encryption + simplified MiF)
============================================
Encryptions + MiF time (enc + T_mif): 1.743090s
Collected 40491 trails

[DEBUG] caught good trail (pair #1470357)
[DEBUG] caught good trail (pair #4118409)
[DEBUG] caught good trail (pair #21612141)
[DEBUG] caught good trail (pair #24813673)
[DEBUG] caught good trail (pair #31692675)
[DEBUG] caught good trail (pair #48175110)
[DEBUG] caught good trail (pair #53784802)

More right pairs than expected, direct attack may take a bit longer (current version does not detect it)

Stage 2 (multi-trail recursive procedure)
=========================================
progress: k0 0/16 trails 6525
progress: k0 1/16 trails 6583
progress: k0 2/16 trails 6584
progress: k0 3/16 trails 6537
progress: k0 4/16 trails 6543
progress: k0 5/16 trails 6544
progress: k0 6/16 trails 6548
progress: k0 7/16 trails 6460
progress: k0 8/16 trails 6525
!!! Key recovered and confirmed 1: 322d cb17 f68f f089
Multi-trail recurse time (T_cnt): 0.617728s
```

## Example 12 rounds

```bash
$ make test_attack12
time ./attack12.elf 29.4 4
n_rounds: 12
seed: 1528124126 = 0x5b1552de
c: 4
n_pairs: 708405415 = 2^29.40
[DEBUG] Secret master key: bc88 248e b3c9 4c6e  | round keys: bc88 2af3 1597 a471 d9e7 f6f8 893a ef26 02d4 be21 af37 f267 

Stage 0+1 (data encryption + simplified MiF)
============================================
Encryptions + MiF time (enc + T_mif): 41.330857s
Collected 12161179 trails

[DEBUG] caught good trail (pair #44921456)
[DEBUG] caught good trail (pair #67537646)
[DEBUG] caught good trail (pair #97125355)
[DEBUG] caught good trail (pair #319819412)
[DEBUG] caught good trail (pair #340219261)
[DEBUG] caught good trail (pair #346792919)
[DEBUG] caught good trail (pair #357253254)
[DEBUG] caught good trail (pair #620321140)

More right pairs than expected, direct attack may take a bit longer (current version does not detect it)

Stage 2 (multi-trail recursive procedure)
=========================================
progress: k0 0/16 trails 1996546
progress: k0 1/16 trails 1996504
progress: k0 2/16 trails 1996920
progress: k0 3/16 trails 1995705
progress: k0 4/16 trails 1995697
progress: k0 5/16 trails 1995413
progress: k0 6/16 trails 1996276
progress: k0 7/16 trails 1994868
!!! Key recovered and confirmed 1: bc88 248e b3c9 4c6e
Multi-trail recurse time (T_cnt): 300.917031s

# full search takes 600-700s
```

