import { PrivateKey } from '../src/index.js';
import { KeyPair } from '../src/symbol/index.js';
import fs from 'fs';

export const readContents = filepath => fs.readFileSync(filepath, { encoding: 'utf8', flag: 'r' });
export const readPrivateKey = filepath => new KeyPair(new PrivateKey(readContents(filepath).trim()));
