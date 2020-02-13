MKDIR_P = mkdir -p
OUT_DIR = obj/server obj/client build

.PHONY: directories
all: directories

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

.PHONY: all
all:
	cd libs/inih/extra && $(MAKE) -f Makefile.static
	cd client && $(MAKE)
	cd server && $(MAKE)
.PHONY: clean

clean:
	rm -rf ${OUT_DIR}

