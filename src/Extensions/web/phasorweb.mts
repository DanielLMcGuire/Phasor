import { serve } from 'zorvix';
import { fileURLToPath } from 'node:url';
import { resolve } from 'node:path';
import { existsSync } from 'node:fs';
import type { AddressInfo } from 'node:net';

const exeName = process.platform === 'win32' ? 'phasor.exe' : 'phasor';
const exePath = resolve(process.cwd(), 'phasor', 'bin', exeName);

if (!process.env.API_KEY) {
    console.error("API_KEY is missing!");
    process.exit(1);
}

if (!existsSync(exePath)) {
    console.error(`phasor executable not found at ${exePath}! Please clone and build it using the sandbox preset!`);
    process.exit(1);
}

if (resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
    serve({ port: 62811, logging: false, workers: true }, async (server) => {
        // @ts-ignore // This is executed by the worker at ./node_modules/zorvix/dist, which ts could not possibly know
        const { registerRoutes } = await import('../../../dist/routes.min.mjs');
        registerRoutes(server);
        await server.start();
        console.log(`Phasor is live at http://0.0.0.0:${(server.server.address() as AddressInfo).port}`);
    });
}
