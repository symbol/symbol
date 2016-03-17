import {Parser} from 'binary-parser';

export function convertPing(input) {
	const parser = new Parser()
		.endianess('little')
		.uint32('size')
		.uint32('type')
		.uint32('heartbeat');
	let ping = parser.parse(input);
	delete ping['size'];
	return ping;
}
