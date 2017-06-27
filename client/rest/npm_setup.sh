#!/bin/sh

# install sdk
cd catapult-sdk
npm install
npm link
npm run rebuild
cd ..

# install servers
for module in 'monitor' 'rest' 'spammer'
do
	cd "${module}"
	npm install
	npm uninstall catapult-sdk
	npm link catapult-sdk
	cd ..
done
