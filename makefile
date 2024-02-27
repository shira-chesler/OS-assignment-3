.PHONY: all clean

all:
	$(MAKE) -C A all
	$(MAKE) -C B all
	$(MAKE) -C C all

clean:
	$(MAKE) -C A clean
	$(MAKE) -C B clean
	$(MAKE) -C C clean
