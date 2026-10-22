# osc_src -- rebuild os_full.bin from the reconstructed sources.
#
#   make          generate (if needed), assemble, link, verify
#   make verify   compare the rebuilt image with the dump
#   make stats    report the code/data split and the asm/bytes split
#   make clean
#
# Inputs are read from ../dumps/os_full.bin; everything is written under
# osc_src/.  Needs arm-none-eabi binutils and capstone (segment.py only).

PY      ?= python3
AS      ?= arm-none-eabi-as
IMAGE   := ../dumps/os_full.bin
OUT     := code/build/os_full.bin
CACHE   := build/seg.pkl
FUNCS   := build/functions.txt
OBJS    := build/objects.txt
JOBS    ?= $(shell nproc 2>/dev/null || echo 4)

.PHONY: all verify gen assemble link stats clean selftest

all:
	@test -f $(CACHE)  || $(PY) tools/segment.py $(IMAGE) --cache $(CACHE)
	@test -f $(FUNCS)  || $(PY) tools/gen_functions.py --cache $(CACHE) -o $(FUNCS)
	@test -d code/asm   || $(PY) tools/gen_asm.py --cache $(CACHE) --functions $(FUNCS) -o code/asm
	@test -f data/data_00.s || $(PY) tools/gen_data.py --cache $(CACHE) -o data
	$(PY) tools/rebuild.py --jobs $(JOBS) -o $(OUT) --reference $(IMAGE)

verify:
	$(PY) tools/verify.py $(OUT) $(IMAGE)

selftest:
	$(PY) tools/selftest_link.py

stats:
	$(PY) tools/segment.py $(IMAGE)
	$(PY) tools/stats.py

clean:
	rm -rf build/obj build/as code/build data/*.s data/*.c data/*.bin
	rm -f $(OUT)

distclean: clean
	rm -rf code/asm build/seg.pkl build/functions.txt build/objects.txt
