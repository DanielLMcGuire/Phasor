import { serve } from 'zorvix';
import { fileURLToPath } from 'node:url';
import { resolve } from 'node:path';
import { existsSync } from 'node:fs';

const exeName = process.platform === 'win32' ? 'phasor.exe' : 'phasor';
const exePath = resolve(process.cwd(), 'phasor', 'bin', exeName);
process.env["PHASOR_NO_ENV"] = '1';

if (!process.env.API_KEY) {
    console.error("API_KEY is missing!");
    process.exit(1);
}

if (!existsSync(exePath)) {
    console.error(`phasor executable not found at ${exePath}! Please clone and build it using the sandbox preset!`);
    process.exit(1);
}

if (resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
    // When workers: true, serve() requires the absolute path to the setup module.
    // The primary process will fork a worker which imports this path automatically.
    const setupModulePath = resolve(process.cwd(), 'dist', 'routes.min.mjs');
    serve({ port: 62811, logging: false, workers: true }, setupModulePath);
}