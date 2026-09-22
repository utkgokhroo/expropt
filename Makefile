CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC = src/main.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/symbol_table.cpp src/semantic_analyzer.cpp
TARGET = expropt

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
