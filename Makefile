#
#
#

.PHONY: all stdlib clean cleantmp
.PHONY: compiler compiler_clean engine_clean houken houken-clean
.PHONY: setup-clean setup-unix setup-ds setup-limit-psp
.PHONY: test test_summary
.PHONY: parsertest_clean parsertest
.PHONY: commontest_clean commontest enginetest_clean enginetest
.PHONY: hyunit_clean hyunit compilertest_clean compilertest
.PHONY: tools tools_clean mmorg mmsjis


all:	test_summary

clean:	compiler_clean engine_clean parsertest_clean commontest_clean enginetest_clean hyunit_clean compilertest_clean houken-clean houkensample-clean  setup-clean tools_clean

cleantmp:
	find . \( -name ".#*" -o -name "*.stackdump" \) -print | xargs rm -f

houken:
	@echo "Entering directory" \``pwd`/houken\'
	(cd houken && rake)
	@echo "Leaving directory" \``pwd`/houken\'

houken-clean:
	(cd houken && rake clean)

houkensample-clean:
	(cd houken/sample && rake clean)

stdlib:
	@echo "Entering directory" \``pwd`/stdlib\'
	(cd stdlib && rake)
	@echo "Leaving directory" \``pwd`/stdlib\'

compiler:	setup-unix
	@echo "Entering directory" \``pwd`/compiler\'
	(cd compiler && rake)
	@echo "Leaving directory" \``pwd`/compiler\'

compiler_clean:
	(cd compiler && rake clean)

engine_clean:
	-$(MAKE) -C engine clean


test:	setup-unix parsertest_clean parsertest commontest_clean commontest compilertest_clean compilertest enginetest_clean enginetest hyunit_clean hyunit

parsertest_clean:
	-(cd parser/test && rake clean)

parsertest:
	@echo "Entering directory" \``pwd`/parser/test\'
	-(cd parser/test && rake)
	@echo "Leaving directory" \``pwd`/parser/test\'

commontest_clean:
	-$(MAKE) -C common/test clean

commontest:
	$(MAKE) -C common/test

enginetest_clean:
	-$(MAKE) -C engine/test clean

enginetest:
	$(MAKE) -C engine/test

hyunit_clean:
	-(cd hyunit/test && rake clean)

hyunit:
	@echo "Entering directory" \``pwd`/hyunit/test\'
	(cd hyunit/test && rake run)
	@echo "Leaving directory" \``pwd`/hyunit/test\'

compilertest_clean:
	-$(MAKE) -C compiler/test clean

compilertest:	stdlib
	$(MAKE) -C compiler/test


test_summary:
	$(MAKE) test 2>&1 | ruby ./testSummarize.rb


tools:
	@echo "Entering directory" \``pwd`/tools\'
	(cd tools && rake)
	@echo "Leaving directory" \``pwd`/tools\'

tools_clean:
	(cd tools && rake clean)

mmorg:
	ruby common/mmesex.rb common > common/mmorg.txt
	ruby common/mmesex.rb compiler > compiler/mmorg.txt
	ruby common/mmesex.rb engine stdlib/ffi > engine/mmorg.txt

mmsjis:
	ruby -Ks common/mmescv.rb common/mm_sjis.txt compiler/mm_sjis.txt > compiler/mm_sjis.mm
	ruby -Ks common/mmescv.rb common/mm_sjis.txt engine/mm_sjis.txt > engine/mm_sjis.mm



setup-ds:
	ruby ./setup-machdep.rb machdep/ds

setup-unix:
	ruby ./setup-machdep.rb machdep/unix

setup-limit-psp:
	ruby ./setup-machdep.rb machdep/limit-psp

setup-clean:
	rm -f common/machdep.h
	rm -f common/machdep.cpp
	rm -f compiler/machdep.h
	rm -f compiler/machdep.cpp
	rm -f compiler/sysDep.h
	rm -f compiler/sysDep.cpp
	rm -f compiler/MACHDEP
	rm -f engine/machdep.h
	rm -f engine/machdep.cpp
	rm -f engine/MACHDEP
