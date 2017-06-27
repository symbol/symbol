#!/bin/sh

for module in 'catapult-sdk' 'monitor' 'rest' 'spammer'
do
	cd "${module}"
	npm $@
	cd ..
done
