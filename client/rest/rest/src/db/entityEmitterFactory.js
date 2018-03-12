const EventEmitter = require('events');
const winston = require('winston');

module.exports = {
	createEntityEmitter: createOpEmitter => {
		const entityEmitter = new EventEmitter();
		return createOpEmitter({ ns: 'catapult.blocks', op: 'i' })
			.then(opEmitter => {
				opEmitter.on('op', doc => {
					entityEmitter.emit('block', doc.o);
				});
				opEmitter.on('error', err => {
					winston.error('detected error watching blocks', err);
					entityEmitter.emit('error', err);
				});
			})
			.then(() => entityEmitter);
	}
};
