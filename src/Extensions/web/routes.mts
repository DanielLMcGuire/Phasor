// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import { spawn } from 'node:child_process';
import { resolve } from 'node:path';
import type { IncomingMessage } from 'node:http';
import type { AddressInfo } from 'node:net';
import type { ServerInstance } from 'zorvix';
import { createBodyParser } from 'zorvix';

async function runViaPipe(
    executablePath: string,
    code: string
): Promise<{ stdout: string; stderr: string; exitCode: number }> {
    return new Promise((resolve) => {
        const proc = spawn(executablePath, [], { stdio: ['pipe', 'pipe', 'pipe'] });

        let stdout = ''; let stderr = '';
        proc.stdout.setEncoding('utf8'); proc.stderr.setEncoding('utf8');
        proc.stdout.on('data', (chunk) => { stdout += chunk; });
        proc.stderr.on('data', (chunk) => { stderr += chunk; });
        proc.on('close', (exitCode) => {
            resolve({
                stdout: stdout.replace(/\r\n/g, '\n'),
                stderr: stderr.replace(/\r\n/g, '\n'),
                exitCode: exitCode ?? -1
            });
        });

        proc.on('error', (err) => {
            console.error(err);
            resolve({
                stdout,
                stderr: 'An internal server error occured. If you are a server administrator, please see the server logs for more information.',
                exitCode: -1
            });
        });

        proc.stdin.write(code, 'utf8');
        proc.stdin.end();
    });
}

export default async function (server: ServerInstance) {
    server.use('/run', createBodyParser({ limit: 2 * 1048576 }));
    server.use("/run",  (req, res, next) => {
        const APIKEY =
        typeof req.query.apikey === 'string'
            ? req.query.apikey
            : req.headers['x-api-key'];

        if (!APIKEY || APIKEY !== process.env.API_KEY) {
            res.json({ error: 'Unauthorized' }, 400);
            return;
        }

        next();
    });

    server.post('/run', async (req, res) => {
        if (parseInt(req.headers['content-length'] || '0', 10) > 1024 * 128) {
            res.json({ error: 'Code too large' }, 413);
            return;
        }
        .
        const code = req.body as string;
        if (!code || typeof code !== 'string' || !code.trim()) { 
            res.json({ error: 'Request body must contain source code.' }, 400); 
            return; 
        }

        const exeName = process.platform === 'win32' ? 'phasor.exe' : 'phasor';
        const exePath = resolve(process.cwd(), 'phasor', 'bin', exeName);

        res.json(await runViaPipe(exePath, code));
    });

    server.get('/version', async (req, res) => {
        const exeName = process.platform === 'win32' ? 'phasor.exe' : 'phasor';
        const exePath = resolve(process.cwd(), 'phasor', 'bin', exeName);

        res.json({ version: `${(await runViaPipe(exePath, 'using("stdmeta");print(phs_version());')).stdout.trim()}` });
    });

    await server.start();
    console.log(`Phasor is live at http://0.0.0.0:${(server.server.address() as AddressInfo).port}`);
}