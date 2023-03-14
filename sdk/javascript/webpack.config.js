import WasmPackPlugin from '@wasm-tool/wasm-pack-plugin';
import webpack from 'webpack';
import path from 'path';
import URL from 'url';

export default {
	entry: './src/index.js',
	mode: process.env.NODE_ENV || 'development',
	output: {
		path: path.resolve(path.dirname(URL.fileURLToPath(import.meta.url)), '_build'),
		filename: 'bundle.js',
		library: {
			type: 'module'
		}
	},

	resolve: {
		extensions: ['.js'],
		fallback: {
			crypto: 'crypto-browserify',
			stream: 'stream-browserify'
		}
	},

	plugins: [
		new webpack.ProvidePlugin({
			process: 'process/browser',
			Buffer: ['buffer', 'Buffer']
		}),
		new WasmPackPlugin({
			crateDirectory: path.resolve(path.dirname(URL.fileURLToPath(import.meta.url)), 'wasm'),
			extraArgs: '--no-typescript',
			outName: 'symbol_crypto_wasm',
			outDir: path.resolve(path.dirname(URL.fileURLToPath(import.meta.url)), '_build')
		}),
		new webpack.NormalModuleReplacementPlugin(/..\/..\/wasm\/pkg\/symbol_crypto_wasm/, '../../_build/symbol_crypto_wasm.js')
	],

	experiments: {
		asyncWebAssembly: true,
		outputModule: true
	}
};
