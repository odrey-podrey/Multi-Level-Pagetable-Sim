CC = clang
CFLAGS = -pedantic -std=c11 -fsanitize=address -g -Wall
LDFLAGS = -pedantic -fsanitize=address -g -lm
LIB = libmlpt.a

all: libmlpt.a

# runme: libmlpt.a pagetable_sim.o
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o runme pagetable_sim.o $(LIB)
# I didn't include this because the instructions say to export the API in mlpt.h, and our default target should be libmlpt.a,
# so I'm assuming there's no executable and no need for linking here in particular. 

libmlpt.a: pagetable_sim.o 
	ar rcs $(LIB) pagetable_sim.o 

pagetable_sim.o: pagetable_sim.c mlpt.h config.h
	$(CC) $(CFLAGS) -c pagetable_sim.c 

clean: 
	rm --force $(LIB) pagetable_sim.o