# Compilador y flags
CXX = g++
CXXFLAGS = -Wall -Iinclude -std=c++17

# Carpetas
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Archivos
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET = $(BIN_DIR)/menu

# Regla principal
all: $(TARGET)

# Enlazar
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compilar
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/menu

# Ejecutar
run: all
	@export $(grep -v '^#' .env | xargs); \
	./$(TARGET) -u ivan -p 1234 -f Libros/archivo.txt
