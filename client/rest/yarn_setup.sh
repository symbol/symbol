#!/bin/sh

# install servers
for module in 'catapult-sdk' 'rest' 'spammer' 'tools'
do
	cd "${module}"
	yarn install
	cd ..
done
