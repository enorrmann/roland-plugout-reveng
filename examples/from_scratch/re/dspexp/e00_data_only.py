# Control: do ARM writes to DT40's RAW view stay (program untouched, unused words 0x700-0x70f)?
PROG = None
DATA = {0x700 + i: float(i + 1) for i in range(16)}
READ = (0x700, 16)
