CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC = src/main.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/symbol_table.cpp src/semantic_analyzer.cpp src/tac.cpp src/tac_generator.cpp src/tac_optimizer.cpp
HDR = $(wildcard include/*.h)
TARGET = expropt

$(TARGET): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
