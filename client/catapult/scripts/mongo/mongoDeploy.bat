@echo off

set PATH="c:\Program Files\MongoDB\Server\3.2\bin";%PATH%

IF NOT EXIST mongoDbPath.txt (
  echo create MONGODBPATH.TXT with a path to mongo db storage
  goto :eof
)

set /p MONGO_DBPATH=<mongoDbPath.txt
start mongod.exe --dbpath %MONGO_DBPATH% --replSet catapult0 --oplogSize 100

rem sleep
ping -n 3 -w 500 127.1 > NUL

mongo --eval "rs.initiate()" local

rem sleep
ping -n 3 -w 500 127.1 > NUL

mongo catapult < mongoDbPrepare.js > NUL

rem sleep
ping -n 3 -w 500 127.1 > NUL

echo "checking oplog"
mongo --eval "db.oplog.rs.find({}, {ts: 1}).sort({ts: -1}).limit(10)" local

echo "shutting down"
mongo --eval "db.getSiblingDB('admin').shutdownServer({ force: true })"

