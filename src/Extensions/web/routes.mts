import { spawn } from 'node:child_process';
import { resolve } from 'node:path';
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

// In cluster mode with workers: true, zorvix dynamically imports this file
// and passes the ServerInstance to the default export.
export default async function setup(server: ServerInstance) {
    server.use('/run', createBodyParser({ limit: 2 * 1048576 } as any));
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
        if (parseInt(req.headers['content-length'] || '0', 10) > 1024 * 128) { // 128KB max
            res.json({ error: 'Code too large' }, 413);
            return;
        }
        
        // In the new API, req.body is fully parsed by the middleware, so it does not need to be awaited.
        const code = req.body as string;
        if (!code || typeof code !== 'string' || !code.trim()) { 
            res.json({ error: 'Request body must contain source code.' }, 400); 
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
    
    // server.port reflects the configured port. 0 indicates testing environments
    if (server.port !== 0) {
        console.log(`Phasor is live at http://0.0.0.0:${(server.server.address() as AddressInfo).port}`);
    }
}