CXXFLAGS += -std=c++17 -Iinclude -Isrc
ARFLAGS +=
LDFLAGS +=

SRCS = $(wildcard src/*.cc)
OBJS = $(SRCS:src/%.cc=out/%.o)

EXAMPLE_SRCS = $(wildcard examples/*.cc)
EXAMPLE_OBJS = $(EXAMPLE_SRCS:examples/%.cc=out/examples/%.o)
EXAMPLE_BINS = $(EXAMPLE_SRCS:examples/%.cc=bin/examples/%)


all: libthreadpp.a

examples: $(EXAMPLE_BINS)


libthreadpp.a: $(OBJS)
	mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

out/%.o: src/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $^

out/examples/%.o: examples/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $^

bin/examples/%: out/examples/%.o libthreadpp.a
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf bin out libthreadpp.a
