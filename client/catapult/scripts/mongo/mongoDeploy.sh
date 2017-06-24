#!/bin/bash

RUNNING_INSTANCES=$(ps -ax | grep "mongod " | grep -v grep | wc -l)
mongo --eval "db.runCommand( { serverStatus: 1 } )" local > /dev/null 2>&1
if [ $RUNNING_INSTANCES -eq 1 -o $? -eq 0 ]; then
	echo "Make sure mongod is stopped prior to running the script"
	exit 1
fi

MONGO_DBPATH=""

if [ -f /etc/mongod.conf ]; then
	echo "/etc/mongod.conf found, should I use db path from it?"

	select yn in "Yes" "No"; do
		case $yn in
			Yes )
				MONGO_DBPATH=$(cat /etc/mongod.conf | grep dbPath | cut -d: -f2 | xargs)
				DIR_OWNER=$(stat -c '%U' "${MONGO_DBPATH}")
				CURRENT_USER=$(whoami)
				if [ "$CURRENT_USER" != "$DIR_OWNER" ]; then
					echo "If you want to use system config, you must run via sudo like 'sudo -u ${DIR_OWNER} $0 $*'";
					exit
				fi
				break;;
			No )
				break;;
		esac
	done
fi

if [ -z "${MONGO_DBPATH}" ]; then
	if [ ! -f mongoDbPath.txt ]; then
		echo "File not found!"
		exit 1
	fi

	MONGO_DBPATH=$(cat mongoDbPath.txt)
fi

echo "starting mongod in background, ${MONGO_DBPATH}"
mongod --dbpath "${MONGO_DBPATH}" --replSet catapult0 --oplogSize 100 > /tmp/mongod-log.txt 2>&1 &

sleep 1
while true;
do
	mongo --eval "db.runCommand( { serverStatus: 1 } )" local > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		break;
	fi
	echo "waiting for mongod start..."
	sleep 1
done

mongo --eval "rs.initiate()" local

sleep 1

mongo catapult < mongoDbPrepare.js > /dev/null

sleep 1

echo "checking oplog"
mongo --eval "db.oplog.rs.find({}, {ts: 1}).sort({ts: -1}).limit(10)" local

echo "shutting down"
mongo --eval "db.getSiblingDB('admin').shutdownServer({ force: true })"

echo ""
echo "IMPORTANT"
echo ""
if [ -n "${DIR_OWNER}" ]; then
	echo "you need to manually add following configuration to /etc/mongod.conf "
	echo ""
	cat << EOF
replication:
	oplogSizeMB: 100
	replSetName: catapult0
EOF
else
	echo "remember to start the mongod server with following arguments"
	echo "  --dbpath '${MONGO_DBPATH}' --replSet catapult0 --oplogSize 100"
fi
