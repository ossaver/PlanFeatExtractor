CC = g++
# Final version: remove -g and replace -O0 by -O3
CFLAGS = -c -Wall -std=c++11 -O3
LFLAGS = -Wall -std=c++11 -O3
OBJS = planFeatExtractor.o parser.o syntaxAnalyzer.o parsedTask.o preprocess.o preprocessedTask.o grounder.o groundedTask.o mutex.o stages.o features.o task.o

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o planFeatExtractor
	
planFeatExtractor.o:
	$(CC) $(CFLAGS) planFeatExtractor.cpp

parser.o:
	$(CC) $(CFLAGS) parser/parser.cpp

syntaxAnalyzer.o:
	$(CC) $(CFLAGS) parser/syntaxAnalyzer.cpp

parsedTask.o:
	$(CC) $(CFLAGS) parser/parsedTask.cpp
	
preprocess.o:
	$(CC) $(CFLAGS) preprocess/preprocess.cpp
	
preprocessedTask.o:
	$(CC) $(CFLAGS) preprocess/preprocessedTask.cpp

grounder.o:
	$(CC) $(CFLAGS) grounder/grounder.cpp

groundedTask.o:
	$(CC) $(CFLAGS) grounder/groundedTask.cpp

mutex.o:
	$(CC) $(CFLAGS) stages/mutex.cpp

features.o:
	$(CC) $(CFLAGS) stages/features.cpp

stages.o:
	$(CC) $(CFLAGS) stages/stages.cpp

task.o:
	$(CC) $(CFLAGS) stages/task.cpp

clean:
	rm -f *.o
	rm -f planFeatExtractor

