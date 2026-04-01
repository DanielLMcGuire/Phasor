import { spawn } from 'node:child_process';
import { resolve } from 'node:path';
import type { IncomingMessage } from 'node:http';
import type { ServerInstance } from 'zorvix';

function readBody(req: IncomingMessage): Promise<string> {
    return new Promise((res, rej) => {
        const chunks: Buffer[] = [];
        req.on('data', (c) => chunks.push(c));
        req.on('end',  ()  => res(Buffer.concat(chunks).toString('utf8')));
        req.on('error', rej);
    });
}

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

export function registerRoutes(server: ServerInstance) {
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
        const code = await readBody(req);
        if (!code.trim()) { res.json({ error: 'Request body must contain source code.' }, 400); return; }

        const exeName = process.platform === 'win32' ? 'phasor.exe' : 'phasor';
        const exePath = resolve(process.cwd(), 'phasor', 'bin', exeName);

        res.json(await runViaPipe(exePath, code));
    });

    server.get('/version', async (req, res) => {
        res.json({ version: '3.1.1', build: 'windows-64-rel_sandbox' });
    });
}