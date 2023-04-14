import WasmPackPlugin from '@wasm-tool/wasm-pack-plugin';
import webpack from 'webpack';
import path from 'path';
import URL from 'url';

const target = 'web';
const buildDirectory = path.resolve(path.dirname(URL.fileURLToPath(import.meta.url)), '_build');
const distDirectory = path.resolve(path.dirname(URL.fileURLToPath(import.meta.url)), 'dist');

export default {
	entry: './src/index.js',
	mode: process.env.NODE_ENV || 'development',
	target,
	devtool: 'source-map',

	output: {
		path: distDirectory,
		filename: `bundle.${target}.js`,
		library: { type: 'module' }
	},

	// add plugins and resolvers for setting up node to browser mappings
	plugins: [
		new WasmPackPlugin({
			crateDirectory: path.resolve(path.dirname(URL.fileURLToPath(import.meta.url)), 'wasm'),
			target,
			extraArgs: '--no-typescript',
			outName: 'symbol_crypto_wasm',
			outDir: `${buildDirectory}/wasm/${target}_webpack`
		}),
		new webpack.ProvidePlugin({
			process: 'process/browser',
			Buffer: ['buffer', 'Buffer']
		}),
		new webpack.NormalModuleReplacementPlugin(
			/symbol-crypto-wasm-node/,
			`../../_build/wasm/${target}_webpack/symbol_crypto_wasm.js`
		)
	],
	resolve: {
		extensions: ['.js'],
		fallback: {
			crypto: 'crypto-browserify',
			stream: 'stream-browserify'
		}
	},

	experiments: {
		asyncWebAssembly: true,
		outputModule: true
	}
};
