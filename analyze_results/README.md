# Complexity Computations for Attacks on Speck32/Speck64

The computations take as input a file with distributions of weights of truncated trails, which are placed in the folder [distrib](./distrib/) (here, *dx/dy* means the input difference and *cx/cy* means the output difference of the upper `r`-round differential (if any):


```bash
> ls -1 distrib/
speck32_rounds11_split1-0-8-2_cw24_dx0a20_dy4205.txt
speck32_rounds11_split1-0-8-2_cw32_dx0a60_dy4205.txt
speck32_rounds11_split1-0-8-2_cw37_dx0a20_dy4205.txt
speck32_rounds11_split1-6-2-2_cw25_dx0a20_dy4205_cx8000_cy840a.txt
speck32_rounds11_split1-6-2-2_cw34_dx0211_dy0a04_cx850a_cy9520.txt
speck32_rounds12_split1-0-9-2_cw38_dx0a20_dy4205.txt
speck32_rounds12_split1-7-2-2_cw36_dx0a20_dy4205_cx850a_cy9520.txt
speck32_rounds12_split1-8-1-2_cw31_dx7458_dyb0f8_cx802a_cyd4a8.txt
speck32_rounds13_split1-0-10-2_cw43_dx7448_dyb0f8.txt
speck32_rounds13_split1-8-2-2_cw40_dx7448_dyb0f8_cx850a_cy9520.txt
speck32_rounds14_split1-9-2-2_cw50_dx8054_dya900_cx0040_cy0542.txt
speck32_rounds15_split1-10-2-2_cw55_dx2800_dy0010_cx0004_cy0014.txt
speck64_rounds13_split1-8-2-2_cw55_dx820200_dy1202_cx20200000_cy1206008.txt
speck64_rounds13_split1-8-2-2_cw56_dx820200_dy1202_cx20200000_cy1206008.txt
speck64_rounds13_split1-8-2-2_cw59_dx820200_dy1202_cx20200000_cy1206008.txt
speck64_rounds19_split1-14-2-2_cw81_dx4092400_dy20040104_cx80008004_cy84008020.txt
speck64_rounds19_split1-14-2-2_cw86_dx4092400_dy20040104_cx80008004_cy84008020.txt
speck64_rounds20_split1-15-2-2_cw92_dx4092400_dy20040104_cx808080a0_cya08481a4.txt
````

These distributions were used to compute the attacks in Table 3, Table 4 and Table 5 of the paper, with corrected Differential IDs.

When called on one of the files, the main script [analyze.py](./analyze.py) produces detailed information about all possible attacks (with `c` ranging from 1 to 8).

The summaries of all attacks, as produced by the script, are saved in the files [summary_Speck32.txt](./summary_Speck32.txt) and [summary_Speck64.txt](./summary_Speck64.txt).