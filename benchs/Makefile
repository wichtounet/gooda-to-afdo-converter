BENCHS = foo bar

define BENCH_TEST

BENCHS_TEST += $(1)_test
ALL += $(1)_test base_results_test_$(1) second_results_test_$(1)

$(1)_test: base_results_test_$(1) second_results_test_$(1)

base_results_test_$(1):
	echo "here"

second_results_test_$(1):
	echo "there"

endef

#Generate targets
$(foreach bench,$(BENCHS),$(eval $(call BENCH_TEST,$(bench))))

bench_test: $(BENCHS_TEST)
	echo "end"

.PHONY: bench_test $(ALL)
