const symbolSdk = require('../src/index');
const fs = require('fs');

const readContents = filepath => fs.readFileSync(filepath, { encoding: 'utf8', flag: 'r' });
const readPrivateKey = filepath => new symbolSdk.symbol.KeyPair(new symbolSdk.CryptoTypes.PrivateKey(readContents(filepath).trim()));

module.exports = { readContents, readPrivateKey };
