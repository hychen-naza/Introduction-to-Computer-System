PHI_CC = /opt/mpss/3.3/sysroots/x86_64-mpsssdk-linux/usr/libexec/k1om-mpss-linux/gcc/k1om-mpss-linux/4.7.0/gcc
HOST_CC = gcc 
PHI_AR = /opt/mpss/3.3/sysroots/x86_64-mpsssdk-linux/usr/libexec/k1om-mpss-linux/gcc/k1om-mpss-linux/4.7.0/ar
HOST_AR = ar

ifeq ($(TARGET),phi)
  CC=$(PHI_CC) -g -Wall -O3
  AR=$(PHI_AR)
else
  CC=$(HOST_CC) -m64 -g -Wall -O3
  AR=$(HOST_AR)
  TARGET=host
endif



all: libfilter.a p_band_scan pthread-ex parallel-sum-ex


libfilter.a : filter.o signal.o timing.o
	$(AR) ruv libfilter.a filter.o signal.o timing.o

filter.o : filter.c filter.h
	$(CC) -msse3 -march=native -ftree-vectorizer-verbose=7 -c filter.c -fopenmp 
	#$(CC) -msse3 -march=native -ftree-vectorizer-verbose=7 -ffast-math -c filter.c -fopenmp 
	#$(CC) -O3 -mavx -ftree-vectorizer-verbose=5 -msse3 -ffast-math -c filter.c -fopenmp 
	#$(CC) -c filter.c -fopenmp 

signal.o : signal.c signal.h
	$(CC) -c signal.c

timing.o : timing.c timing.h
	$(CC) -c timing.c


band_scan: band_scan.c filter.h signal.h timing.h libfilter.a
	$(CC) band_scan.c -L. -lfilter -lm -o band_scan -fopenmp

p_band_scan: p_band_scan.c filter.h signal.h timing.h libfilter.a
	$(CC) -pthread p_band_scan.c -L. -lfilter -lm -o p_band_scan -fopenmp
#
# Your rule for p_band_scan will look like the 
# following.  Note the use of the -pthread option
# which is critical
#
#p_band_scan: p_band_scan.c filter.h signal.h timing.h libfilter.a 
#	    $(CC) -pthread p_band_scan.c -L. -lfilter -lm -o p_band_scan
#

clean-filter: 
	-rm filter.o signal.o timing.o libfilter.a  band_scan 2>/dev/null || true

.PHONY: clean-filter

parallel-sum-ex: parallel-sum-ex.c
	$(CC) -pthread parallel-sum-ex.c -o parallel-sum-ex

pthread-ex: pthread-ex.c
	$(CC) -pthread pthread-ex.c -o pthread-ex

clean-examples:
	-rm -f pthread-ex parallel-sum-ex 2>/dev/null || true

.PHONY: clean-examples


clean: clean-filter clean-examples

