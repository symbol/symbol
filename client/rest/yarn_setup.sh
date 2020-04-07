#!/bin/sh

# install sdk
cd catapult-sdk
yarn install
yarn run rebuild
cd ..

# install servers
for module in 'rest' 'spammer' 'tools'
do
	cd "${module}"
	yarn install
	cd ..
done
