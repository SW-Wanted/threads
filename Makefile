CC      = cc
CFLAGS  = -Wall -Wextra -Werror -g -fsanitize=thread
LFLAGS  = -lpthread

# All phases in order
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
# ─────────────────────────────────────────────

all:
	@for p in $(PHASES); do \
		for src in phases/$$p/*.c; do \
			[ -f "$$src" ] || continue; \
			bin="$${src%.c}"; \
			$(CC) $(CFLAGS) "$$src" -o "$$bin" $(LFLAGS) \
				&& printf "  \033[32mOK\033[0m  $$bin\n" \
				|| printf "  \033[31mFAIL\033[0m  $$src\n"; \
		done; \
	done

# ─────────────────────────────────────────────
# Compile a single phase:  make phase=01
# ─────────────────────────────────────────────

ifdef phase
PHASE_DIR = $(firstword $(foreach p,$(PHASES),$(if $(findstring _0$(phase)_,_$(p)_),$(p))))
ifeq ($(PHASE_DIR),)
PHASE_DIR = $(firstword $(foreach p,$(PHASES),$(if $(findstring $(phase),$(p)),$(p))))
endif

one_phase:
	@if [ -z "$(PHASE_DIR)" ]; then \
		echo "Phase '$(phase)' not found. Available: 00 01 02 03 04 05 06 07 08 09"; \
		exit 1; \
	fi
	@for src in phases/$(PHASE_DIR)/*.c; do \
		[ -f "$$src" ] || continue; \
		bin="$${src%.c}"; \
		$(CC) $(CFLAGS) "$$src" -o "$$bin" $(LFLAGS) \
			&& printf "  \033[32mOK\033[0m  $$bin\n" \
			|| printf "  \033[31mFAIL\033[0m  $$src\n"; \
	done
endif

# ─────────────────────────────────────────────
# Compile and run one exercise:  make run ex=phases/phase_01_lifecycle/ex01
# ─────────────────────────────────────────────

ifdef ex
run:
	@$(CC) $(CFLAGS) "$(ex).c" -o "$(ex)" $(LFLAGS) \
		&& printf "  \033[32mOK\033[0m  $(ex)\n" \
		|| (printf "  \033[31mFAIL\033[0m  $(ex).c\n"; exit 1)
	@"./$(ex)"
endif

# ─────────────────────────────────────────────
# Remove all compiled binaries
# ─────────────────────────────────────────────

clean:
	@for p in $(PHASES); do \
		for src in phases/$$p/*.c; do \
			[ -f "$$src" ] || continue; \
			bin="$${src%.c}"; \
			[ -f "$$bin" ] && rm -f "$$bin" && printf "  rm  $$bin\n"; \
		done; \
	done

# ─────────────────────────────────────────────
# Print a summary of all phases and their exercises
# ─────────────────────────────────────────────

list:
	@for p in $(PHASES); do \
		printf "\n\033[1m$$p\033[0m\n"; \
		for src in phases/$$p/*.c; do \
			[ -f "$$src" ] && printf "    $$src\n"; \
		done; \
	done
	@echo ""

.PHONY: all clean list run one_phase