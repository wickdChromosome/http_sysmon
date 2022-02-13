#!/bin/bash

set -eou pipefail

if [ $1 == "compile" ]; then

	echo "Compiling project.."
	cd /src/build &&
		cmake .. &&
		make

else

	echo "Running project.."
	/src/build/bin/project

fi	


