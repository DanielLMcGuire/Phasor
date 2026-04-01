import assert from 'node:assert/strict';
import type { AddressInfo } from 'node:net';
import { test, before, after } from 'node:test';
import { createServer } from 'zorvix';
import { registerRoutes } from '#phasorweb/routes';
import type { ServerInstance } from 'zorvix';

const SOURCE = "using(\"stdio\"); var x = 15; var y = 12; var z = x * y; putf(\"%d * %d = %d\", x, y, z);";
const EXPECTED = {"stdout":"15 * 12 = 180\n","stderr":"","exitCode":0};

let server: ServerInstance;
let baseUrl: string;

before(async () => {
    server = createServer({ port: 0, logging: false });
    registerRoutes(server);
    await server.start();
    const { port } = server.server.address() as AddressInfo;
    baseUrl = `http://localhost:${port}`;
});

after(async () => {
    await server.stop();
});

async function post(path: string, body: string) {
    const res = await fetch(`${baseUrl}${path}`, {
        method:  'POST',
        headers: { 'Content-Type': 'text/plain', 'X-API-Key': process.env.API_KEY ?? '' },
        body,
    });
    return { status: res.status, body: await res.json() };
}

test('POST /run — returns 200 with json body', async () => {
    const { status } = await post('/run', SOURCE);
    assert.equal(status, 200);
});

test('POST /run — stdout matches expected', async () => {
    const { body } = await post('/run', SOURCE);
    assert.equal(body.stdout, EXPECTED.stdout);
});

test('POST /run — stderr matches expected', async () => {
    const { body } = await post('/run', SOURCE);
    assert.equal(body.stderr, EXPECTED.stderr);
});

test('POST /run — exit code matches expected', async () => {
    const { body } = await post('/run', SOURCE);
    assert.equal(body.exitCode, EXPECTED.exitCode);
});

test('POST /run — empty body returns 400', async () => {
    const { status } = await post('/run', '   ');
    assert.equal(status, 400);
});