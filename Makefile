# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
#                       Matthias Kretz <m.kretz@gsi.de>

all:

prefix=installed

build:
	@cmake -B $@ -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=$(prefix)

distclean:
	@rm -r build

test: all

help clean install test all: build
	@cmake --build $< --target $@

docs: Doxyfile include/vir/val.h README.md
	@doxygen
