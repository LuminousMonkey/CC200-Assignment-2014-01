.PHONY: all

build:
	cd src; \
	    cnet ASSIGNMENT.MAP


all: clean
	cnet ASSIGNMENT.MAP

clean:
	rm -f *.o *.cnet
	cd src; \
	    rm *.o *.cnet
