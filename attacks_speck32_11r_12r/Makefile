all: attack11.elf attack12.elf

attack11.elf: mifattack.cpp speck.cpp recursive.cpp common.h
	g++ mifattack.cpp recursive.cpp speck.cpp -DSPECK32_ATTACK=11 -std=c++17 -march=native -O3 -o attack11.elf

attack12.elf: mifattack.cpp speck.cpp recursive.cpp common.h
	g++ mifattack.cpp recursive.cpp speck.cpp -DSPECK32_ATTACK=12 -std=c++17 -march=native -O3 -o attack12.elf

test_attack11: attack11.elf
	time ./attack11.elf 26.1 3

test_attack12: attack12.elf
	time ./attack12.elf 29.4 4

clean:
	rm -f attack11.elf attack12.elf