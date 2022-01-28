#!/bin/bash

set -ex

mongod --fork --logpath /data/mongod.log

wait_period=10
while ! /usr/bin/mongo --eval "printjson(db.version())" >/dev/null 2>&1; do
	sleep 1
	((--wait_period))
	if [ "$wait_period" -le 1 ]; then
		echo "Waiting for monogo timed out, exiting now.."
		exit 1
	fi
done
