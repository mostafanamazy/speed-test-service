MKDIR_P = mkdir -p
OUT_DIR = obj build

.PHONY: directories
all: directories

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

.PHONY: all
all:
	cd client && $(MAKE)
	cd server && $(MAKE)
.PHONY: clean

clean:
	rm -rf build obj

