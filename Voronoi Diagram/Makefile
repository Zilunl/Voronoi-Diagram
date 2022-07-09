# define C compiler & flags
CC = gcc
CFLAGS = -Wall -g

# define libraries to be linked (for example -lm)
LIB = -lm


HDR = intersect.h euclid_space.h geometry.h tower.h dynamic_array.h utils.h
SRC = intersect.c euclid_space.c geometry.c tower.c dynamic_array.c utils.c voronoi2.c 
OBJ     = $(SRC:.c=.o)

# the executable name
EXE = voronoi2

all: $(EXE)

$(EXE): $(HDR) $(OBJ) Makefile
	echo $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LIB)

#VAL = valgrind -v --leak-check=full --show-reachable=yes
VAL =
#VAL = valgrind -v --leak-check=full --show-reachable=yes --track-origins=yes
#VAL = valgrind --tool=memcheck --suppressions=./ed.supp -v --leak-check=full --show-reachable=yes 
t0: $(EXE)
	$(VAL) ./$(EXE) s5.csv poly5.txt out0.txt < cut0.txt
	cat ./out0.txt

t1: $(EXE)
	$(VAL) ./$(EXE) s5.csv poly5.txt out1.txt < cut51.txt
	cat ./out1.txt

t2: $(EXE)
	$(VAL) ./$(EXE) s5.csv poly5.txt out2.txt < cut52.txt
	cat ./out2.txt

t4: $(EXE)
	$(VAL) ./$(EXE) s5.csv poly5.txt out4.txt < cut53.txt
	cat ./out4.txt

t: $(EXE)
	$(VAL) ./$(EXE) s10ds.csv poly4.txt out5.txt < cut4.txt
	cat ./out5.txt

clean: 
	rm -f $(OBJ) $(EXE)

$(OBJ): $(HDR)
