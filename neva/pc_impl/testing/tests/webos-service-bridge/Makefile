NAME="app.webosservicebridge"

all: agl-package webos-package

webos-package:
	@which ares-package || { echo "No 'ares-package' found, see https://www.webosose.org/docs/tools/sdk/cli/cli-user-guide/#cli-commands"; exit 1; }
	ares-package -e *.wgt -e *.ipk .

agl-package:
	zip -r ${NAME}.wgt * -x *.wgt *.ipk \*~

.PHONY: clean
clean:
	rm -rf *.wgt *.ipk
