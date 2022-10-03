import symbolSdk from '../src/index.js';
import fs from 'fs';

export const readContents = filepath => fs.readFileSync(filepath, { encoding: 'utf8', flag: 'r' });
export const readPrivateKey = filepath => new symbolSdk.symbol.KeyPair(new symbolSdk.PrivateKey(readContents(filepath).trim()));
