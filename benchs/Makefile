BENCHS = tonto bwaves milc sjeng h264ref astarBigLakes
BUILD_BENCHS = build_tonto build_bwaves build_milc build_sjeng build_h264ref build_astarBigLakes

#Benchmark testing
BENCHS_REF = tonto_ref bwaves_ref milc_ref sjeng_ref h264ref_ref astarBigLakes_ref
BENCHS_TEST = tonto_test bwaves_test milc_test sjeng_test h264ref_test astarBigLakes_test

#Overhead testing
OVERHEADS_REF = overhead_ref_tonto overhead_ref_bwaves overhead_ref_milc overhead_ref_sjeng overhead_ref_h264ref overhead_ref_astarBigLakes
OVERHEADS_TEST = overhead_test_tonto overhead_test_bwaves overhead_test_milc overhead_test_sjeng overhead_test_h264ref overhead_test_astarBigLakes

bench_ref: $(BENCHS_REF)
	cat log_ref_base.* > log_base
	cat log_ref_instrumentation.* > log_instrumentation
	cat log_ref_ucc.* > log_ucc
	cat log_ref_lbr.* > log_lbr
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

build: $(BUILD_BENCHS)

define BENCH_REF

$(1)_ref: base_results_ref_$(1) instrumentation_results_ref_$(1) ucc_results_ref_$(1) lbr_results_ref_$(1)

base_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_base $(1) > log_ref_base.$(1)

instrumentation_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_instrumentation $(1) > log_ref_instrumentation.$(1)

ucc_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_sampling_ucc $(1) > log_ref_ucc.$(1)

lbr_results_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_sampling_lbr $(1) > log_ref_lbr.$(1)

endef

define BENCH_TEST

$(1)_test: base_results_test_$(1) instrumentation_results_test_$(1) ucc_results_test_$(1) lbr_results_test_$(1)

base_results_test_$(1):
	+runspec --username=wichtounet --action=validate ---iterations=1 --size=test -config=gcc_google_47_base $(1) > log_test_base.$(1)

instrumentation_results_test_$(1):
	+runspec --username=wichtounet --action=validate ---iterations=1 --size=test --config=gcc_google_47_instrumentation $(1) > log_test_instrumentation.$(1)

ucc_results_test_$(1):
	+runspec --username=wichtounet --action=validate ---iterations=1 --size=test --config=gcc_google_47_sampling_ucc $(1) > log_test_ucc.$(1)

lbr_results_test_$(1):
	+runspec --username=wichtounet --action=validate ---iterations=1 --size=test --config=gcc_google_47_sampling_lbr $(1) > log_test_lbr.$(1)

endef

define OVERHEAD_REF

overhead_ref_$(1): base_overhead_ref_$(1) instrumentation_overhead_ref_$(1) ucc_overhead_ref_$(1) lbr_overhead_ref_$(1)

base_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_base $(1) > overhead_ref_base.$(1)

instrumentation_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_overhead_instrumentation $(1) > overhead_ref_instrumentation.$(1)

ucc_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_overhead_sampling_ucc $(1) > overhead_ref_ucc.$(1)

lbr_overhead_ref_$(1):
	+runspec --username=wichtounet --action=validate --iterations=3 --size=ref --config=gcc_google_47_overhead_sampling_lbr $(1) > overhead_ref_lbr.$(1)

endef

define OVERHEAD_TEST

overhead_test_$(1): base_overhead_test_$(1) instrumentation_overhead_test_$(1) ucc_overhead_test_$(1) lbr_overhead_test_$(1)

base_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --config=gcc_google_47_base $(1) > overhead_test_base.$(1)

instrumentation_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --config=gcc_google_47_overhead_instrumentation $(1) > overhead_test_instrumentation.$(1)

ucc_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --config=gcc_google_47_overhead_sampling_ucc $(1) > overhead_test_ucc.$(1)

lbr_overhead_test_$(1):
	+runspec --username=wichtounet --action=validate --iterations=1 --size=test --config=gcc_google_47_overhead_sampling_lbr $(1) > overhead_test_lbr.$(1)

endef

define BUILD

build_$(1): build_base_$(1) build_instrumentation_$(1) build_ucc_$(1) build_lbr_$(1)

build_base_$(1):	
	+runspec --username=wichtounet --action=build --config=gcc_google_47_base $(1) > build_log_base.$(1)
	
build_instrumentation_$(1):	
	+runspec --username=wichtounet --action=build --config=gcc_google_47_instrumentation $(1) > build_log_instrumentation.$(1)
	
build_ucc_$(1):	
	+runspec --username=wichtounet --action=build --config=gcc_google_47_sampling_ucc $(1) > build_log_ucc.$(1)
	
build_lbr_$(1):	
	+runspec --username=wichtounet --action=build --config=gcc_google_47_sampling_lbr $(1) > build_log_lbr.$(1)
	
endef

#Generate targets
$(foreach bench,$(BENCHS),$(eval $(call BUILD,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call BENCH_REF,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call BENCH_TEST,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call OVERHEAD_REF,$(bench))))
$(foreach bench,$(BENCHS),$(eval $(call OVERHEAD_TEST,$(bench))))
