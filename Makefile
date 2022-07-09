CXXFLAGS = -O2
LDFLAGS =
ARFLAGS = rv

COMMON_CXXFLAGS = -std=c++17 -pthread -Iinclude -Isrc
COMMON_LDFLAGS = -pthread

SRCS = $(shell find src -name *.cc | sort)
OBJS = $(SRCS:src/%.cc=out/%.o)
DEPS = $(SRCS:src/%.cc=out/%.d)
SRC_CXXFLAGS =
SRC_DEPFLAGS = -MT $@ -MMD -MP -MF out/$*.d
SRC_ARFLAGS =

EXAMPLE_SRCS = $(shell find examples -name *.cc | sort)
EXAMPLE_OBJS = $(EXAMPLE_SRCS:%.cc=out/%.o)
EXAMPLE_DEPS = $(EXAMPLE_SRCS:%.cc=out/%.d)
EXAMPLE_BINS = $(EXAMPLE_SRCS:%.cc=bin/%)
EXAMPLE_CXXFLAGS =
EXAMPLE_DEPFLAGS = -MT $@ -MMD -MP -MF out/examples/$*.d
EXAMPLE_LDFLAGS = libthreadpp.a


.PHONY: all
all: libthreadpp.a

-include $(DEPS) $(EXAMPLE_DEPS)

.PHONY: examples
examples: $(EXAMPLE_BINS)


libthreadpp.a: $(OBJS)
	mkdir -p $(@D)
	$(AR) $(ARFLAGS) $(SRC_ARFLAGS) $@ $^

out/%.o: src/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(COMMON_CXXFLAGS) $(SRC_CXXFLAGS) $(SRC_DEPFLAGS) -c -o $@ $<


out/examples/%.o: examples/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(COMMON_CXXFLAGS) $(EXAMPLE_CXXFLAGS) $(EXAMPLE_DEPFLAGS) -c -o $@ $<

bin/examples/%: out/examples/%.o libthreadpp.a
	mkdir -p $(@D)
	$(CXX) -o $@ $< $(LDFLAGS) $(COMMON_LDFLAGS) $(EXAMPLE_LDFLAGS)


.PHONY: clean
clean:
	rm -rf bin out libthreadpp.a
