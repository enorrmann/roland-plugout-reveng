# Which of 1e05 / 0c03 reads memory and which writes it?
# If 1e05 = load (r0, r1 <- [400], [401]) and 0c03 = store: [410] = 1, [411] = 2, [412]/[413] keep 19/20.
# If 1e05 = store and 0c03 = load: [400]/[401] and [412]/[413] get register values.
PROG = '''
0a02 ff70
1e05 0fe0 1fe2 0400 0401
0001 0001 0001
0c03 0fe0 0410
0c03 1fe0 0411
0001 0001 0001
1e05 0fe0 1fe2 0412 0413
0a02 ff70 0b02 00f8 0001
'''
DATA = {0x400 + i: float(i + 1) for i in range(32)}
READ = (0x400, 32)
