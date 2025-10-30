#compilador y flags
cxx = g++
cxxflags = -wall -std=c++17 -Iinclude -pthread -O2

#carpetas
src_dir = src
bin_dir = bin

#nombres de ejecutables
exec_menu = menu
exec_admin = admin
exec_multi_matrices = multi_matrices
exec_crea_idx = creaidx
exec_crea_indice = creaindiceinvertido
exec_crea_indice_par = creaindiceinvertidoparalelo
exec_game_server = game_server
exec_game_client = game_client

#rutas completas de ejecutables
target_menu = $(bin_dir)/$(exec_menu)
target_admin = $(bin_dir)/$(exec_admin)
target_multi_matrices = $(bin_dir)/$(exec_multi_matrices)
target_crea_idx = $(bin_dir)/$(exec_crea_idx)
target_crea_indice = $(bin_dir)/$(exec_crea_indice)
target_crea_indice_par = $(bin_dir)/$(exec_crea_indice_par)
target_game_server = $(bin_dir)/$(exec_game_server)
target_game_client = $(bin_dir)/$(exec_game_client)

#regla principal: compilar todo
all: $(target_menu) $(target_admin) $(target_multi_matrices) \
     $(target_crea_idx) $(target_crea_indice) $(target_crea_indice_par) \
     $(target_game_server) $(target_game_client)
	@echo "todos los ejecutables han sido compilados"

#crear carpeta bin si no existe
$(bin_dir):
	mkdir -p $(bin_dir)

#reglas individuales de compilacion
$(target_menu): $(src_dir)/menu.cpp $(src_dir)/utils.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $(src_dir)/menu.cpp $(src_dir)/utils.cpp -o $@

$(target_admin): $(src_dir)/main.cpp $(src_dir)/utils.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $(src_dir)/main.cpp $(src_dir)/utils.cpp -o $@

$(target_multi_matrices): $(src_dir)/multiplica_mat.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_crea_idx): $(src_dir)/crear_idx.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_crea_indice): $(src_dir)/crear_indice_invertido.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

#nueva: creaindiceinvertidoparalelo (usa hilos)
$(target_crea_indice_par): $(src_dir)/crea_indice_invertido_paralelo.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_game_server): $(src_dir)/game_server.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_game_client): $(src_dir)/game_client.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

#limpiar binarios
clean:
	rm -f $(bin_dir)/*

#cargar variables de entorno
setup:
	@echo "ejecute los siguientes comandos manualmente en su terminal:"
	@echo "export USER_FILE=\"usuarios.txt\""
	@echo "export PERFIL_FILE=\"perfiles.txt\""
	@echo "export ADMIN_SYS=\"$(target_admin)\""
	@echo "export MULTI_M=\"$(target_multi_matrices)\""
	@echo "export CREATE_INDEX=\"$(target_crea_indice)\""
	@echo "export CREATE_IDX=\"$(target_crea_idx)\""
	@echo "export INDICE_INVET_PARALELO=\"$(target_crea_indice_par)\""
	@echo "export N_THREADS=4"
	@echo "export N_LOTE=4"
	@echo ""
	@echo "#variables del juego"
	@echo "export GAME_SERVER=\"$(target_game_server)\""
	@echo "export GAME_CLIENT=\"$(target_game_client)\""
	@echo "export PORT=4000"
	@echo "export GAME_BOARD_X=50"
	@echo "export POS_VICTORIA_C=50"
	@echo "export DICE_R=6"
	@echo "export MIN_TEAMS=2"
	@echo "export MAX_TEAMS=4"
	@echo "export MIN_PLAYERS=2"
	@echo "export MAX_PLAYERS=5"
	@echo "variables exportadas correctamente"

.PHONY: all clean setup
