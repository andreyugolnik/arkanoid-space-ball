VER_MAJOR=1
VER_MINOR=3
VER_RELEASE=5
VERSION=$(VER_MAJOR).$(VER_MINOR)$(VER_RELEASE)
BUILD_DIR_RELEASE=.build_release
BUILD_DIR_DEBUG=.build_debug
BUNDLE_NAME=arkanoidsb

PREFIX?=/usr/local

UNAME=$(shell uname -s)
ifeq ($(UNAME), Darwin)
	BUNDLE_NAME=arkanoidsb.app
endif

all:
	@echo "Usage:"
	@echo "    make <release | debug>    - make release or debug application"
	@echo "    make <cppcheck>           - do static code verification"
	@echo "    make <clean>              - cleanup directory"

package:
	cd tools/rescompiler && make && cd ../../res && ../tools/rescompiler/rescompiler arkanoidsb

release: package
	$(shell if [ ! -d $(BUILD_DIR_RELEASE) ]; then mkdir $(BUILD_DIR_RELEASE); fi)
	cd $(BUILD_DIR_RELEASE) ; cmake -DCMAKE_BUILD_TYPE=Release -DAPP_VERSION_MAJOR:STRING=$(VER_MAJOR) -DAPP_VERSION_MINOR:STRING=$(VER_MINOR) -DAPP_VERSION_RELEASE:STRING=$(VER_RELEASE) .. ; make ; cd ..
	cp -r $(BUILD_DIR_RELEASE)/$(BUNDLE_NAME) .

debug: package
	$(shell if [ ! -d $(BUILD_DIR_DEBUG) ]; then mkdir $(BUILD_DIR_DEBUG); fi)
	cd $(BUILD_DIR_DEBUG) ; cmake -DCMAKE_BUILD_TYPE=Debug -DAPP_VERSION_MAJOR:STRING=$(VER_MAJOR) -DAPP_VERSION_MINOR:STRING=$(VER_MINOR) -DAPP_VERSION_RELEASE:STRING=$(VER_RELEASE) .. ; make ; cd ..
	cp -r $(BUILD_DIR_DEBUG)/$(BUNDLE_NAME) .

cppcheck:
	cppcheck -j 1 --enable=all -f -I src src/ 2> cppcheck-output

clean:
	rm -fr $(BUILD_DIR_RELEASE) $(BUILD_DIR_DEBUG) $(BUNDLE_NAME) cppcheck-output $(BUNDLE_NAME)-$(VERSION)* $(BUNDLE_NAME)_$(VERSION)* *.{log,tasks,sh,xz,list} strace_out cov-int
