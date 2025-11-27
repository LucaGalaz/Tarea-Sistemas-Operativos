#compilador y flags
cxx = g++
cxxflags = -Wall -std=c++17 -Iinclude -pthread -O2

#carpetas
src_dir = src
bin_dir = bin
log_dir = logs
stats_dir = stats

#nombres de ejecutables
exec_menu = menu
exec_admin = admin
exec_multi_matrices = multi_matrices
exec_crea_idx = creaidx
exec_crea_indice = creaIndiceInvertido
exec_crea_indice_par = creaIndiceInvertidoParalelo
exec_game_server = game_server
exec_game_client = game_client
exec_benchmark = benchmark
exec_stats = stats_generator.py
exec_buscador = buscador_SistOpe
exec_motor = motor_busqueda
exec_cache = cache

#rutas completas de ejecutables
target_menu = $(bin_dir)/$(exec_menu)
target_admin = $(bin_dir)/$(exec_admin)
target_multi_matrices = $(bin_dir)/$(exec_multi_matrices)
target_crea_idx = $(bin_dir)/$(exec_crea_idx)
target_crea_indice = $(bin_dir)/$(exec_crea_indice)
target_crea_indice_par = $(bin_dir)/$(exec_crea_indice_par)
target_game_server = $(bin_dir)/$(exec_game_server)
target_game_client = $(bin_dir)/$(exec_game_client)
target_benchmark = $(bin_dir)/$(exec_benchmark)
target_buscador = $(bin_dir)/$(exec_buscador)
target_motor = $(bin_dir)/$(exec_motor)
target_cache = $(bin_dir)/$(exec_cache)

# Cargar variables desde .env si existe
ifneq (,$(wildcard .env))
    include .env
    export $(shell sed 's/=.*//' .env)
endif

#regla principal: compilar todo
all: $(target_menu) $(target_admin) $(target_multi_matrices) \
     $(target_crea_idx) $(target_crea_indice) $(target_crea_indice_par) \
     $(target_game_server) $(target_game_client) $(target_benchmark) $(target_buscador) \
	 $(target_motor) $(target_cache)
	@echo "todos los ejecutables han sido compilados"

#crear carpetas bin, logs y stats si no existen
$(bin_dir) $(log_dir) $(stats_dir):
	mkdir -p $@

#reglas individuales de compilacion
$(target_menu): $(src_dir)/menu.cpp $(src_dir)/utils.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $(src_dir)/menu.cpp $(src_dir)/utils.cpp -o $@

$(target_admin): $(src_dir)/main.cpp $(src_dir)/utils.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $(src_dir)/main.cpp $(src_dir)/utils.cpp -o $@

$(target_multi_matrices): $(src_dir)/multiplica_mat.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_crea_idx): $(src_dir)/crear_IDX.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_crea_indice): $(src_dir)/crear_indice_invertido.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_crea_indice_par): $(src_dir)/crea_indice_invertido_paralelo.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

# CORRECCIÓN 1: El servidor necesita la carpeta logs antes de ejecutarse
$(target_game_server): $(src_dir)/game_server.cpp | $(bin_dir) $(log_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_game_client): $(src_dir)/game_client.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

# CORRECCIÓN 2: El benchmark necesita la carpeta stats antes de ejecutarse
$(target_benchmark): $(src_dir)/benchmark_runner.cpp | $(bin_dir) $(stats_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_buscador): $(src_dir)/buscador_SistOpe.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_motor): $(src_dir)/motor_busqueda.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

$(target_cache): $(src_dir)/cache.cpp | $(bin_dir)
	$(cxx) $(cxxflags) $< -o $@

#limpiar binarios, logs y estadísticas (solo el contenido de las carpetas)
clean:
	# Borra el contenido de bin/
	rm -f $(bin_dir)/*

	# Borra el contenido de logs/
	rm -f $(log_dir)/*

	# Borra el contenido de stats/

	rm -f $(stats_dir)/*

	@echo "Limpieza completada: Se borró el contenido de bin/, logs/ y stats/."

#cargar variables de entorno (AGREGADAS LAS VARIABLES NUEVAS)
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
	@echo "export GAME_LOG_FILE=\"$(log_dir)/game_log.csv\""
	@echo ""
	@echo "#variables de estadisticas (Problema 2.B)"
	@echo "export STATS_APP=\"python3 $(src_dir)/$(exec_stats)\""
	@echo "export STATS_OUTPUT_DIR=\"$(stats_dir)/game_plots\""
	@echo ""
	@echo "#variables de benchmark (Problema 2.A)"
	@echo "export BENCHMARK_APP=\"$(target_benchmark)\""
	@echo "export PLOT_SCRIPT=\"src/plot_performance.py\""
	@echo "export STATS_FOLDER=\"stats\""
	@echo "variables exportadas correctamente"

.PHONY: all clean setup