#BENCHS = tonto bwaves milc sjeng 105.h264Forebase 106.h264Foremain 107.h264Sss 001.astarBigLakes 002.astarRivers lbm 005.bzip2Program 006.bzip2Combined calculix povray omnetpp 102.soplexRef 103.soplexPds50 libquantum xalancbmk

#BENCHS = libquantum xalancbmk calculix 103.soplexPds50 005.bzip2Program sjeng 107.h264Sss lbm
BENCHS =

define BENCH_REF

BENCHS_REF += $(1)_ref
ALL += $(1)_ref base_results_ref_$(1) instrumentation_results_ref_$(1) ucc_results_ref_$(1) lbr_results_ref_$(1)

$(1)_ref: base_results_ref_$(1) instrumentation_results_ref_$(1) ucc_results_ref_$(1) lbr_results_ref_$(1)

base_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_base $(1) > log_ref_base.$(1)

instrumentation_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --config=gcc_google_47_instrumentation $(1) > log_ref_instrumentation.$(1)

ucc_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --config=gcc_google_47_sampling_ucc $(1) > log_ref_ucc.$(1)

lbr_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --config=gcc_google_47_sampling_lbr $(1) > log_ref_lbr.$(1)

endef

define BENCH_REF_FAST

BENCHS_REF_FAST += $(1)_ref_fast
ALL += $(1)_ref_fast base_results_ref_fast_$(1) instrumentation_results_ref_fast_$(1) ucc_results_ref_fast_$(1) lbr_results_ref_fast_$(1)

$(1)_ref_fast: base_results_ref_fast_$(1) instrumentation_results_ref_fast_$(1) ucc_results_ref_fast_$(1) lbr_results_ref_fast_$(1)

base_results_ref_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --config=gcc_google_47_base $(1) > log_ref_fast_base.$(1)

instrumentation_results_ref_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --config=gcc_google_47_instrumentation $(1) > log_ref_fast_instrumentation.$(1)

ucc_results_ref_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --config=gcc_google_47_sampling_ucc $(1) > log_ref_fast_ucc.$(1)

lbr_results_ref_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --config=gcc_google_47_sampling_lbr $(1) > log_ref_fast_lbr.$(1)

endef

define BENCH_CHEAT

BENCHS_CHEAT += $(1)_cheat
ALL += $(1)_cheat base_results_cheat_$(1) instrumentation_results_cheat_$(1) ucc_results_cheat_$(1) lbr_results_cheat_$(1)

$(1)_cheat: base_results_cheat_$(1) instrumentation_results_cheat_$(1) ucc_results_cheat_$(1) lbr_results_cheat_$(1)

base_results_cheat_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_base $(1) > log_cheat_base.$(1)

instrumentation_results_cheat_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --train_with=train_ref --config=gcc_google_47_instrumentation $(1) > log_cheat_instrumentation.$(1)

ucc_results_cheat_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --train_with=train_ref --config=gcc_google_47_sampling_ucc $(1) > log_cheat_ucc.$(1)

lbr_results_cheat_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --train_with=train_ref --config=gcc_google_47_sampling_lbr $(1) > log_cheat_lbr.$(1)

endef

define BENCH_CHEAT_FAST

BENCHS_CHEAT_FAST += $(1)_cheat_fast
ALL += $(1)_cheat base_results_cheat_fast_$(1) instrumentation_results_cheat_fast_$(1) ucc_results_cheat_fast_$(1) lbr_results_cheat_fast_$(1)

$(1)_cheat_fast: base_results_cheat_fast_$(1) instrumentation_results_cheat_fast_$(1) ucc_results_cheat_fast_$(1) lbr_results_cheat_fast_$(1)

base_results_cheat_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --config=gcc_google_47_base $(1) > log_cheat_fast_base.$(1)

instrumentation_results_cheat_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --train_with=train_ref --config=gcc_google_47_instrumentation $(1) > log_cheat_fast_instrumentation.$(1)

ucc_results_cheat_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --train_with=train_ref --config=gcc_google_47_sampling_ucc $(1) > log_cheat_fast_ucc.$(1)

lbr_results_cheat_fast_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=ref --rebuild --train_with=train_ref --config=gcc_google_47_sampling_lbr $(1) > log_cheat_fast_lbr.$(1)

endef

define BENCH_CHEAT_TEST

BENCHS_CHEAT_TEST += $(1)_cheat_test
ALL += $(1)_cheat base_results_cheat_test_$(1) instrumentation_results_cheat_test_$(1) ucc_results_cheat_test_$(1) lbr_results_cheat_test_$(1)

$(1)_cheat_test: base_results_cheat_test_$(1) instrumentation_results_cheat_test_$(1) ucc_results_cheat_test_$(1) lbr_results_cheat_test_$(1)

base_results_cheat_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_base $(1) > log_cheat_test_base.$(1)

instrumentation_results_cheat_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --train_with=train_ref --config=gcc_google_47_instrumentation $(1) > log_cheat_test_instrumentation.$(1)

ucc_results_cheat_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --train_with=train_ref --config=gcc_google_47_sampling_ucc $(1) > log_cheat_test_ucc.$(1)

lbr_results_cheat_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --train_with=train_ref --config=gcc_google_47_sampling_lbr $(1) > log_cheat_test_lbr.$(1)

endef

define BENCH_TEST

BENCHS_TEST += $(1)_test
ALL += $(1)_test base_results_test_$(1) instrumentation_results_test_$(1) ucc_results_test_$(1) lbr_results_test_$(1)

$(1)_test: base_results_test_$(1) instrumentation_results_test_$(1) ucc_results_test_$(1) lbr_results_test_$(1)

base_results_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_base $(1) > log_test_base.$(1)

instrumentation_results_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_instrumentation $(1) > log_test_instrumentation.$(1)

ucc_results_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_sampling_ucc $(1) > log_test_ucc.$(1)

lbr_results_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_sampling_lbr $(1) > log_test_lbr.$(1)

endef

define OVERHEAD_REF

OVERHEADS_REF += overhead_ref_$1
ALL += overhead_ref_$(1) base_overhead_ref_$(1) instrumentation_overhead_ref_$(1) ucc_overhead_ref_$(1) lbr_overhead_ref_$(1)

overhead_ref_$(1): base_overhead_ref_$(1) instrumentation_overhead_ref_$(1) ucc_overhead_ref_$(1) lbr_overhead_ref_$(1)

base_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_base $(1) > overhead_ref_base.$(1)

instrumentation_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --config=gcc_google_47_overhead_instrumentation $(1) > overhead_ref_instrumentation.$(1)

ucc_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --config=gcc_google_47_overhead_sampling_ucc $(1) > overhead_ref_ucc.$(1)

lbr_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --rebuild --config=gcc_google_47_overhead_sampling_lbr $(1) > overhead_ref_lbr.$(1)

endef

define OVERHEAD_TEST

OVERHEADS_TEST += overhead_test_$1
ALL += overhead_test_$(1) base_overhead_test_$(1) instrumentation_overhead_test_$(1) ucc_overhead_test_$(1) lbr_overhead_test_$(1)

overhead_test_$(1): base_overhead_test_$(1) instrumentation_overhead_test_$(1) ucc_overhead_test_$(1) lbr_overhead_test_$(1)

base_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_base $(1) > overhead_test_base.$(1)

instrumentation_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_overhead_instrumentation $(1) > overhead_test_instrumentation.$(1)

ucc_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_overhead_sampling_ucc $(1) > overhead_test_ucc.$(1)

lbr_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --rebuild --config=gcc_google_47_overhead_sampling_lbr $(1) > overhead_test_lbr.$(1)

endef

#Generate targets
$(foreach bench,$(BENCHS),$(eval $(call BENCH_REF,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call BENCH_REF_FAST,$(bench))))

$(foreach bench,$(BENCHS),$(eval $(call BENCH_TEST,$(bench))))

$(foreach bench,$(BENCHS),$(eval $(call BENCH_CHEAT,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call BENCH_CHEAT_FAST,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call BENCH_CHEAT_TEST,$(bench))))

$(foreach bench,$(BENCHS),$(eval $(call OVERHEAD_REF,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call OVERHEAD_TEST,$(bench))))

bench_ref: $(BENCHS_REF)
	cat log_ref_base.* > log_base
	cat log_ref_instrumentation.* > log_instrumentation
	cat log_ref_ucc.* > log_ucc
	cat log_ref_lbr.* > log_lbr
	bash parse.sh score

bench_ref_fast: $(BENCHS_REF_FAST)
	cat log_ref_fast_base.* > log_base
	cat log_ref_fast_instrumentation.* > log_instrumentation
	cat log_ref_fast_ucc.* > log_ucc
	cat log_ref_fast_lbr.* > log_lbr
	bash parse.sh score

bench_cheat: $(BENCHS_CHEAT)
	cat log_cheat_base.* > log_base
	cat log_cheat_instrumentation.* > log_instrumentation
	cat log_cheat_ucc.* > log_ucc
	cat log_cheat_lbr.* > log_lbr
	bash parse.sh score

bench_cheat_fast: $(BENCHS_CHEAT_FAST)
	cat log_cheat_fast_base.* > log_base
	cat log_cheat_fast_instrumentation.* > log_instrumentation
	cat log_cheat_fast_ucc.* > log_ucc
	cat log_cheat_fast_lbr.* > log_lbr
	bash parse.sh score

bench_cheat_test: $(BENCHS_CHEAT_TEST)
	cat log_cheat_test_base.* > log_base
	cat log_cheat_test_instrumentation.* > log_instrumentation
	cat log_cheat_test_ucc.* > log_ucc
	cat log_cheat_test_lbr.* > log_lbr
	bash parse.sh score

bench_test: $(BENCHS_TEST)
	cat log_test_base.* > log_base
	cat log_test_instrumentation.* > log_instrumentation
	cat log_test_ucc.* > log_ucc
	cat log_test_lbr.* > log_lbr
	bash parse.sh time

overhead_ref: $(OVERHEADS_REF)
	cat overhead_ref_base.* > overhead_base
	cat overhead_ref_instrumentation.* > overhead_instrumentation
	cat overhead_ref_ucc.* > overhead_ucc
	cat overhead_ref_lbr.* > overhead_lbr
	bash parse_overhead.sh time 

overhead_test: $(OVERHEADS_TEST)
	cat overhead_test_base.* > overhead_base
	cat overhead_test_instrumentation.* > overhead_instrumentation
	cat overhead_test_ucc.* > overhead_ucc
	cat overhead_test_lbr.* > overhead_lbr
	bash parse_overhead.sh time

clean:
	runspec --username=wichtounet --action=clobber --config=gcc_google_47_base $(BENCHS) > clean_base
	runspec --username=wichtounet --action=clobber --config=gcc_google_47_instrumentation $(BENCHS) > clean_instrumentation
	runspec --username=wichtounet --action=clobber --config=gcc_google_47_sampling_ucc $(BENCHS) > clean_ucc
	runspec --username=wichtounet --action=clobber --config=gcc_google_47_sampling_lbr $(BENCHS) > clean_lbr

.PHONY: clean overhead_ref overhead_test bench_test bench_ref bench_ref_fast bench_cheat bench_cheat_fast $(ALL)
