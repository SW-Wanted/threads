CC      = cc
CFLAGS  = -Wall -Wextra -Werror -g -fsanitize=thread
LFLAGS  = -lpthread
BUILDDIR = build

PHASES  = phase_00_single_thread \
          phase_01_lifecycle \
          phase_02_data_races \
          phase_03_mutex \
          phase_04_condition_variables \
          phase_05_deadlock \
          phase_06_thread_safety \
          phase_07_cancellation \
          phase_08_advanced

# ─────────────────────────────────────────────
# Default: compile everything
#   make
# ─────────────────────────────────────────────

all:
	@mkdir -p $(BUILDDIR)
	@for p in $(PHASES); do \
		mkdir -p $(BUILDDIR)/$$p; \
		for src in phases/$$p/*.c; do \
			[ -f "$$src" ] || continue; \
			name=$$(basename "$${src%.c}"); \
			bin="$(BUILDDIR)/$$p/$$name"; \
			$(CC) $(CFLAGS) "$$src" -o "$$bin" $(LFLAGS) \
				&& printf "  \033[32mOK\033[0m  $$bin\n" \
				|| printf "  \033[31mFAIL\033[0m  $$src\n"; \
		done; \
	done

# ─────────────────────────────────────────────
# Compile a single phase:
#   make phase=01
#   make phase=03
# ─────────────────────────────────────────────

ifdef phase

PHASE_DIR = $(firstword $(foreach p,$(PHASES),\
              $(if $(filter %_0$(phase)_%,$(p)_),$(p))))
ifeq ($(PHASE_DIR),)
PHASE_DIR = $(firstword $(foreach p,$(PHASES),\
              $(if $(findstring $(phase),$(p)),$(p))))
endif

.DEFAULT_GOAL := _phase

_phase:
	@if [ -z "$(PHASE_DIR)" ]; then \
		echo "Phase '$(phase)' not found."; \
		echo "Available: 00 01 02 03 04 05 06 07 08"; \
		exit 1; \
	fi
	@mkdir -p $(BUILDDIR)/$(PHASE_DIR)
	@printf "\033[1mCompiling $(PHASE_DIR)\033[0m\n"
	@for src in phases/$(PHASE_DIR)/*.c; do \
		[ -f "$$src" ] || continue; \
		name=$$(basename "$${src%.c}"); \
		bin="$(BUILDDIR)/$(PHASE_DIR)/$$name"; \
		$(CC) $(CFLAGS) "$$src" -o "$$bin" $(LFLAGS) \
			&& printf "  \033[32mOK\033[0m  $$bin\n" \
			|| printf "  \033[31mFAIL\033[0m  $$src\n"; \
	done

endif

# ─────────────────────────────────────────────
# Compile and run one exercise:
#   make run ex=phases/phase_01_lifecycle/ex01
# ─────────────────────────────────────────────

ifdef ex

PHASE_OF_EX = $(word 2,$(subst /, ,$(ex)))
NAME_OF_EX  = $(notdir $(ex))

.DEFAULT_GOAL := run

run:
	@mkdir -p $(BUILDDIR)/$(PHASE_OF_EX)
	@bin="$(BUILDDIR)/$(PHASE_OF_EX)/$(NAME_OF_EX)"; \
	$(CC) $(CFLAGS) "$(ex).c" -o "$$bin" $(LFLAGS) \
		&& printf "  \033[32mOK\033[0m  $$bin\n" \
		|| (printf "  \033[31mFAIL\033[0m  $(ex).c\n"; exit 1)
	@./$(BUILDDIR)/$(PHASE_OF_EX)/$(NAME_OF_EX)

endif

# ─────────────────────────────────────────────
# Remove build directory:
#   make clean
# ─────────────────────────────────────────────

clean:
	@rm -rf $(BUILDDIR)
	@printf "  rm  $(BUILDDIR)/\n"

# ─────────────────────────────────────────────
# List all phases and exercises:
#   make list
# ─────────────────────────────────────────────

list:
	@for p in $(PHASES); do \
		printf "\n\033[1m$$p\033[0m\n"; \
		for src in phases/$$p/*.c; do \
			[ -f "$$src" ] && printf "    $$src\n"; \
		done; \
	done
	@echo ""

.PHONY: all clean list run _phase