import MongoDb from 'mongodb';
import EventEmitter from 'events';
import winston from 'winston';

const cursorUtils = {
	getLastDocumentTimestamp: collection =>
		collection
			.find({}, { ts: 1 })
			.sort({ $natural: -1 })
			.limit(1)
			.toArray()
			.then(items => (items.length ? items[0].ts : new MongoDb.Timestamp(0, (Date.now() / 1000) | 0))),

	makeTailable: cursor => {
		for (const flag of ['tailable', 'awaitData', 'oplogReplay', 'noCursorTimeout'])
			cursor.addCursorFlag(flag, true);

		cursor.setCursorOption('numberOfRetries', Number.MAX_VALUE);
	}
};

class TailDataEmitter extends EventEmitter {
	constructor(collection, query) {
		super();
		this.collection = collection;
		this.query = query;
	}

	start() {
		const query = this.query || {};
		return this.getQueryTimestamp()
			.then(queryTs => {
				query.ts = { $gt: queryTs };
				this.cursor = this.collection.find(query);
				cursorUtils.makeTailable(this.cursor);

				this.setStream(this.cursor.stream());
			})
			.catch(err => { this.emit('error', err); });
	}

	stop() {
		if (this.stream) {
			this.stream.destroy();
			this.stream = undefined;
		}

		if (this.cursor) {
			this.cursor.close();
			this.cursor = undefined;
		}
	}

	getQueryTimestamp() {
		return this.ts
			? Promise.resolve(this.ts)
			: cursorUtils.getLastDocumentTimestamp(this.collection);
	}

	setStream(stream) {
		this.stream = stream;
		this.stream.on('data', data => {
			this.ts = data.ts;
			this.emit('op', data);
		});

		this.stream.on('error', err => {
			if (err instanceof MongoDb.MongoError && -1 !== err.message.indexOf('timed out')) {
				winston.warn('restarting after timeout');
				this.stop();
				this.start();
				return;
			}

			this.emit('error', err);
		});

		this.emit('stream', stream);
	}
}

cursorUtils.createTailDataEmitter = (collection, query) => new TailDataEmitter(collection, query);

export default cursorUtils;
