# e01 with bank 5 as input/output (ids 0x3a0.. unused by the stock DSP4 programs).
# If 1e05 = load and 0c03 = store: 3b0 = 1, 3b1 = 2 and 3b2/3b3 keep 19/20.
PROG = '''
0a02 ff70
1e05 0fe0 1fe2 a3a0 a3a1
0001 0001 0001
0c03 0fe0 a3b0
0c03 1fe0 a3b1
0001 0001 0001
1e05 0fe0 1fe2 a3b2 a3b3
0a02 ff70 0b02 00f8 0001
'''
DATA5 = {0x3a0 + i: float(i + 1) for i in range(32)}
READ5 = (0x3a0, 32)
